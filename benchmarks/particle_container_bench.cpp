/*
 * particle_container_bench.cpp
 *
 * Copyright 2026 Holger Schmitz
 *
 * This file is part of Schnek.
 *
 * Schnek is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Schnek is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Schnek.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Synthetic, reproducible PIC-like benchmark for the particle containers (task E1).
//
// Every candidate runs the same deterministic workload. Operations are timed separately: the first append includes
// container construction and first-call allocations, while the other operations are measured after warm-up. Heap
// statistics come from a global operator new/delete hook, so they cover host allocations only. Device memory is
// reported as reserved bytes (capacity times particle size).

#include <malloc.h>

#include <Kokkos_Core.hpp>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#if defined(KOKKOS_ENABLE_CUDA)
#include <cuda_runtime.h>
#endif

#include <decomposition/detail/block_particle_container.hpp>
#include <decomposition/detail/dense_kokkos_particle_container.hpp>
#include <decomposition/detail/device_blocked_particle_container.hpp>
#include <decomposition/detail/particle_factory.hpp>

#ifndef SCHNEK_BENCH_GIT_REVISION
#define SCHNEK_BENCH_GIT_REVISION "unknown"
#endif

#ifndef SCHNEK_BENCH_BUILD_TYPE
#define SCHNEK_BENCH_BUILD_TYPE "unknown"
#endif

namespace {

  // Heap accounting for the global operator new/delete replacements below.
  struct HeapStats {
      std::size_t count = 0;
      std::size_t bytes = 0;
      std::size_t live = 0;
      std::size_t peak = 0;
  };

  HeapStats gHeap;

#if defined(__CUDACC__)
#pragma nv_diagnostic push
// The heap hook is host-only, but nvcc treats replaceable operator new/delete as host-device.
#pragma nv_diag_suppress 20011
#endif

  void *trackedAlloc(std::size_t n) {
    void *p = std::malloc(n == 0 ? 1 : n);
    if (!p) {
      throw std::bad_alloc();
    }
    ++gHeap.count;
    gHeap.bytes += n;
    gHeap.live += malloc_usable_size(p);
    gHeap.peak = std::max(gHeap.peak, gHeap.live);
    return p;
  }

  void trackedFree(void *p) noexcept {
    if (!p) {
      return;
    }
    gHeap.live -= malloc_usable_size(p);
    std::free(p);
  }

}  // namespace

void *operator new(std::size_t n) {
  return trackedAlloc(n);
}

void *operator new[](std::size_t n) {
  return trackedAlloc(n);
}

void operator delete(void *p) noexcept {
  trackedFree(p);
}

void operator delete[](void *p) noexcept {
  trackedFree(p);
}

void operator delete(void *p, std::size_t) noexcept {
  trackedFree(p);
}

void operator delete[](void *p, std::size_t) noexcept {
  trackedFree(p);
}

#if defined(__CUDACC__)
#pragma nv_diagnostic pop
#endif

namespace {

  constexpr double kTwoPi = 6.283185307179586;
  constexpr std::size_t kBins = 64;
  constexpr double kDt = 0.01;
  constexpr double kChargeToMass = -1.0;
  constexpr std::size_t kNotApplicable = static_cast<std::size_t>(-1);

  /// 56-byte particle: position, velocity and weight. Trivially copyable, so the default serialiser applies.
  struct Particle {
      double x[3];
      double v[3];
      double w;
  };

  using Serializer = schnek::ParticleSerializer<Particle>;
  using HostBlock = schnek::BlockParticleContainer<Particle>;
  using DenseHost = schnek::DenseKokkosParticleContainer<Particle, Kokkos::Serial, Kokkos::HostSpace>;
  using DeviceBlockedHost = schnek::DeviceBlockedParticleContainer<Particle, Kokkos::Serial, Kokkos::HostSpace>;
  using DenseDefault = schnek::DenseKokkosParticleContainer<Particle>;
  using DeviceBlockedDefault = schnek::DeviceBlockedParticleContainer<Particle>;

  struct Config {
      std::size_t particles = 200000;
      std::size_t blockCapacity = 256;
      std::size_t fieldCells = 256;
      std::string study;
      int studySteps = 100;
      int steps = 10;
      int repeats = 5;
      std::uint64_t seed = 12345;
      std::vector<std::string> profiles = {"uniform", "clustered"};
      std::string jsonPath;
  };

  void printUsage() {
    std::fprintf(
        stderr,
        "Usage: schnek_particle_bench [options]\n"
        "  --particles N       particles per workload (default 200000)\n"
        "  --block-capacity N  particles per block for blocked containers (default 256)\n"
        "  --steps N           samples for push/deposit (default 10)\n"
        "  --repeats N         samples for append/delete/migrate/rebin (default 5)\n"
        "  --seed N            seed of the deterministic generator (default 12345)\n"
        "  --profile P         uniform, clustered or all (default all)\n"
        "  --field-cells N     cells in the field used by push and deposit (default 256)\n"
        "  --study NAME        run a study instead of the E1 suite; only binning is available\n"
        "  --study-steps N     steps per study run (default 100)\n"
        "  --json PATH         also write one JSON object per result to PATH\n"
    );
  }

  bool parseArgs(int argc, char *argv[], Config &cfg) {
    for (int i = 1; i < argc; ++i) {
      const std::string arg = argv[i];
      auto next = [&]() -> std::string {
        if (i + 1 >= argc) {
          throw std::invalid_argument("missing value for " + arg);
        }
        return argv[++i];
      };
      try {
        if (arg == "--particles") {
          cfg.particles = std::stoull(next());
        } else if (arg == "--block-capacity") {
          cfg.blockCapacity = std::stoull(next());
        } else if (arg == "--steps") {
          cfg.steps = std::stoi(next());
        } else if (arg == "--repeats") {
          cfg.repeats = std::stoi(next());
        } else if (arg == "--seed") {
          cfg.seed = std::stoull(next());
        } else if (arg == "--profile") {
          const std::string p = next();
          if (p == "all") {
            cfg.profiles = {"uniform", "clustered"};
          } else if (p == "uniform" || p == "clustered") {
            cfg.profiles = {p};
          } else {
            throw std::invalid_argument("unknown profile " + p);
          }
        } else if (arg == "--field-cells") {
          cfg.fieldCells = std::stoull(next());
        } else if (arg == "--study") {
          cfg.study = next();
        } else if (arg == "--study-steps") {
          cfg.studySteps = std::stoi(next());
        } else if (arg == "--json") {
          cfg.jsonPath = next();
        } else if (arg == "--help" || arg == "-h") {
          printUsage();
          return false;
        } else {
          throw std::invalid_argument("unknown option " + arg);
        }
      } catch (const std::exception &e) {
        std::fprintf(stderr, "error: %s\n", e.what());
        printUsage();
        return false;
      }
    }
    if (cfg.particles == 0 || cfg.blockCapacity == 0 || cfg.steps < 1 || cfg.repeats < 1) {
      std::fprintf(stderr, "error: particles, block-capacity, steps and repeats must be positive\n");
      return false;
    }
    if (cfg.fieldCells == 0 || cfg.studySteps < 1 || (!cfg.study.empty() && cfg.study != "binning")) {
      std::fprintf(stderr, "error: field-cells and study-steps must be positive, study must be binning\n");
      return false;
    }
    return true;
  }

  // splitmix64 with Box-Muller, written out by hand so the workload does not depend on the standard library's
  // distribution implementations.
  class DeterministicRng {
    public:
      explicit DeterministicRng(std::uint64_t seed) : state_(seed) {}

      std::uint64_t next() {
        std::uint64_t z = (state_ += 0x9E3779B97F4A7C15ULL);
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        return z ^ (z >> 31);
      }

      /// Uniform in [0, 1).
      double uniform() { return static_cast<double>(next() >> 11) * 0x1.0p-53; }

      /// Standard normal deviate.
      double gaussian() {
        const double u1 = 1.0 - uniform();
        const double u2 = uniform();
        return std::sqrt(-2.0 * std::log(u1)) * std::cos(kTwoPi * u2);
      }

    private:
      std::uint64_t state_;
  };

  /**
   * Particles on the periodic unit interval in x. The uniform profile is flat. The clustered profile puts 90% of the
   * particles in a Gaussian blob (sigma 0.02) around x = 0.5, giving a high-contrast density.
   */
  std::vector<Particle> makeParticles(std::size_t n, bool clustered, std::uint64_t seed) {
    DeterministicRng rng(seed);
    std::vector<Particle> parts(n);
    for (auto &p : parts) {
      double x = 0.0;
      if (clustered && rng.uniform() < 0.9) {
        x = 0.5 + 0.02 * rng.gaussian();
        x -= std::floor(x);
      } else {
        x = rng.uniform();
      }
      p.x[0] = x;
      p.x[1] = rng.uniform();
      p.x[2] = rng.uniform();
      p.v[0] = 0.1 * rng.gaussian();
      p.v[1] = 0.1 * rng.gaussian();
      p.v[2] = 0.1 * rng.gaussian();
      p.w = 1.0;
    }
    return parts;
  }

  // Dispatch category of a container, used to pick the kernel launch path.
  enum class Kind { Host, Dense, DeviceBlocked };

  template<class C>
  struct ContainerInfo {
      static constexpr Kind kind = Kind::Host;
      using Exec = Kokkos::Serial;
      using Mem = Kokkos::HostSpace;
      static std::string label() { return "host"; }
  };

  template<class P, class E, class M>
  struct ContainerInfo<schnek::DenseKokkosParticleContainer<P, E, M>> {
      static constexpr Kind kind = Kind::Dense;
      using Exec = E;
      using Mem = M;
      static std::string label() { return std::string(E::name()) + "/" + M::name(); }
  };

  template<class P, class E, class M>
  struct ContainerInfo<schnek::DeviceBlockedParticleContainer<P, E, M>> {
      static constexpr Kind kind = Kind::DeviceBlocked;
      using Exec = E;
      using Mem = M;
      static std::string label() { return std::string(E::name()) + "/" + M::name(); }
  };

  template<class C>
  struct IsBlock : std::false_type {};

  template<class P, class G>
  struct IsBlock<schnek::BlockParticleContainer<P, G>> : std::true_type {};

  template<class C>
  C createContainer(std::size_t blockCapacity) {
    if constexpr (IsBlock<C>::value || ContainerInfo<C>::kind == Kind::DeviceBlocked) {
      return C(blockCapacity);
    } else {
      return C();
    }
  }

  // Classifiers for the traits contract. Bucket 0 keeps a particle, buckets 1 and 2 remove it.
  struct RemoveAll {
      KOKKOS_INLINE_FUNCTION int operator()(const Particle &) const { return 1; }
  };

  struct RemoveLeftStrip {
      KOKKOS_INLINE_FUNCTION int operator()(const Particle &p) const { return p.x[0] < 0.1 ? 1 : 0; }
  };

  struct MigrateClassifier {
      KOKKOS_INLINE_FUNCTION int operator()(const Particle &p) const {
        if (p.x[0] < 0.25) {
          return 1;
        }
        return p.x[0] >= 0.75 ? 2 : 0;
      }
  };

  /// Push with periodic wrap in x and linear interpolation of a 1D field.
  template<class FieldView>
  struct PushInterp {
      FieldView field;
      int cells;

      KOKKOS_INLINE_FUNCTION void operator()(Particle &p) const {
        double x = p.x[0] + p.v[0] * kDt;
        x -= Kokkos::floor(x);
        if (x >= 1.0) {
          x = 0.0;
        }
        p.x[0] = x;

        const double s = x * cells - 0.5;
        const double fl = Kokkos::floor(s);
        const double w = s - fl;
        int i0 = static_cast<int>(fl) % cells;
        if (i0 < 0) {
          i0 += cells;
        }
        const int i1 = (i0 + 1) % cells;
        const double e = (1.0 - w) * field(i0) + w * field(i1);
        p.v[0] += kChargeToMass * e * kDt;
      }
  };

  /// Cloud-in-cell deposition of particle weight with atomic accumulation.
  template<class FieldView>
  struct DepositCic {
      FieldView current;
      int cells;

      KOKKOS_INLINE_FUNCTION void operator()(const Particle &p) const {
        const double s = p.x[0] * cells - 0.5;
        const double fl = Kokkos::floor(s);
        const double w = s - fl;
        int i0 = static_cast<int>(fl) % cells;
        if (i0 < 0) {
          i0 += cells;
        }
        const int i1 = (i0 + 1) % cells;
        Kokkos::atomic_add(&current(i0), p.w * (1.0 - w));
        Kokkos::atomic_add(&current(i1), p.w * w);
      }
  };

  /// Applies f(Particle &) to every particle on the container's execution space.
  template<class C, class F>
  void forEachParticle(C &c, F f) {
    using Info = ContainerInfo<C>;
    if constexpr (Info::kind == Kind::Dense) {
      const std::size_t n = c.size();
      if (n == 0) {
        return;
      }
      Particle *data = c.data();
      Kokkos::parallel_for(
          "bench:forEachDense", Kokkos::RangePolicy<typename Info::Exec>(std::size_t{0}, n),
          KOKKOS_LAMBDA(const std::size_t i) { f(data[i]); }
      );
      Kokkos::fence();
    } else if constexpr (Info::kind == Kind::DeviceBlocked) {
      c.parallelForEach(f);
    } else if constexpr (IsBlock<C>::value) {
      c.forEach([&f](Particle &p) { f(p); });
    } else {
      for (auto &p : c) {
        f(p);
      }
    }
  }

  // Heap accounting window for one measured section.
  struct HeapDelta {
      std::size_t count = 0;
      std::size_t bytes = 0;
      std::size_t peakExtra = 0;
  };

  class HeapWindow {
    public:
      HeapWindow() : count0_(gHeap.count), bytes0_(gHeap.bytes), base_(gHeap.live) { gHeap.peak = gHeap.live; }

      HeapDelta finish() const { return HeapDelta{gHeap.count - count0_, gHeap.bytes - bytes0_, gHeap.peak - base_}; }

    private:
      std::size_t count0_;
      std::size_t bytes0_;
      std::size_t base_;
  };

  template<class Fn>
  double timedMs(Fn &&fn) {
    Kokkos::fence();
    const auto t0 = std::chrono::steady_clock::now();
    fn();
    Kokkos::fence();
    const auto t1 = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(t1 - t0).count();
  }

  struct Sample {
      std::vector<double> ms;
      HeapDelta heap;

      void add(double t, const HeapDelta &h) {
        ms.push_back(t);
        heap = h;
      }

      double median() const {
        std::vector<double> v = ms;
        std::sort(v.begin(), v.end());
        const std::size_t n = v.size();
        return n % 2 == 1 ? v[n / 2] : 0.5 * (v[n / 2 - 1] + v[n / 2]);
      }

      double minimum() const { return *std::min_element(ms.begin(), ms.end()); }
      double maximum() const { return *std::max_element(ms.begin(), ms.end()); }
  };

  // Times fn and records its host-heap statistics into s. Heap statistics are those of the last sample.
  template<class Fn>
  void measure(Sample &s, Fn &&fn) {
    HeapWindow window;
    const double ms = timedMs(fn);
    s.add(ms, window.finish());
  }

  struct Record {
      std::string profile;
      std::string candidate;
      std::string space;
      std::string op;
      std::string note;
      bool supported = true;
      Sample sample;
      std::size_t size = 0;
      std::size_t capacity = 0;
      std::size_t reserved = 0;
      std::size_t reallocs = kNotApplicable;
      std::size_t blocks = kNotApplicable;
      double commBytes = 0.0;
  };

  template<class C>
  void fillContainerDiagnostics(Record &r, const C &c) {
    using Info = ContainerInfo<C>;
    r.size = schnek::ParticleContainerTraits<C>::size(c);
    r.capacity = c.capacity();
    r.reserved = r.capacity * sizeof(Particle);
    if constexpr (Info::kind == Kind::Dense) {
      r.reallocs = c.reallocationCount();
    }
    if constexpr (IsBlock<C>::value || Info::kind == Kind::DeviceBlocked) {
      r.blocks = c.blockCount();
    }
  }

  template<class C>
  void runCandidate(
      const std::string &name,
      const std::string &profile,
      const std::vector<Particle> &parts,
      const Config &cfg,
      std::vector<Record> &out
  ) {
    using Traits = schnek::ParticleContainerTraits<C>;
    using Info = ContainerInfo<C>;
    using Mem = typename Info::Mem;
    using ByteView = Kokkos::View<std::byte *, Mem>;
    using FieldView = Kokkos::View<double *, Mem>;

    const std::size_t n = parts.size();
    const std::size_t pb = Serializer::size();
    const std::string space = Info::label();

    auto emit = [&](const std::string &op, const Sample &s, double comm, const std::string &note) -> Record & {
      Record r;
      r.profile = profile;
      r.candidate = name;
      r.space = space;
      r.op = op;
      r.note = note;
      r.sample = s;
      r.commBytes = comm;
      out.push_back(std::move(r));
      return out.back();
    };

    auto emitNa = [&](const std::string &op, const std::string &note) {
      Record r;
      r.profile = profile;
      r.candidate = name;
      r.space = space;
      r.op = op;
      r.note = note;
      r.supported = false;
      out.push_back(std::move(r));
    };

    auto appendAll = [&](C &c) {
      for (const auto &p : parts) {
        Traits::insert(c, p);
      }
    };

    auto removeAll = [&](C &c) {
      ByteView buf(Kokkos::view_alloc(Kokkos::WithoutInitializing, "bench:remove-all"), n * pb);
      ByteView scratch(
          Kokkos::view_alloc(Kokkos::WithoutInitializing, "bench:scratch"), Traits::partitionScratchBytes(c)
      );
      std::byte *outs[1] = {buf.data()};
      Traits::template partitionAndPack<Serializer>(c, RemoveAll{}, outs, 1, scratch.data());
    };

    auto checkSize = [&](const C &c, const char *op) {
      if (Traits::size(c) != n) {
        throw std::runtime_error(name + ": particle count changed during " + op);
      }
    };

    // 1. First call: construction plus n appends. Each repeat builds a fresh container; the last one is kept.
    std::unique_ptr<C> cp;
    Sample firstAppend;
    for (int r = 0; r < cfg.repeats; ++r) {
      cp.reset();
      measure(firstAppend, [&] {
        cp = std::make_unique<C>(createContainer<C>(cfg.blockCapacity));
        appendAll(*cp);
      });
    }
    C &c = *cp;
    checkSize(c, "append_first");
    fillContainerDiagnostics(emit("append_first", firstAppend, 0.0, "construction + first-call allocations"), c);

    // 2. Steady-state append into a container that was emptied but kept its capacity.
    Sample steadyAppend;
    for (int r = 0; r < cfg.repeats; ++r) {
      removeAll(c);
      measure(steadyAppend, [&] { appendAll(c); });
    }
    checkSize(c, "append_steady");
    fillContainerDiagnostics(emit("append_steady", steadyAppend, 0.0, "after removing all particles"), c);

    // 3. Delete a strip near x = 0. The removed particles are restored untimed so each repeat sees the same workload.
    {
      ByteView buf(Kokkos::view_alloc(Kokkos::WithoutInitializing, "bench:delete"), n * pb);
      ByteView scratch(
          Kokkos::view_alloc(Kokkos::WithoutInitializing, "bench:scratch"), Traits::partitionScratchBytes(c)
      );
      std::byte *outs[1] = {buf.data()};
      std::size_t counts[1] = {0};
      Sample s;
      for (int r = 0; r < cfg.repeats; ++r) {
        HeapWindow window;
        const double ms = timedMs([&] {
          Traits::countBuckets(c, RemoveLeftStrip{}, counts, 1);
          Traits::template partitionAndPack<Serializer>(c, RemoveLeftStrip{}, outs, 1, scratch.data());
        });
        s.add(ms, window.finish());
        Traits::template bulkInsert<Serializer>(c, buf.data(), counts[0]);
      }
      checkSize(c, "delete");
      fillContainerDiagnostics(
          emit("delete", s, 0.0, "removed=" + std::to_string(counts[0]) + " (count + pack; restore untimed)"), c
      );
    }

    // 4. Migration exchange on loopback. Particles leaving the domain are packed into two per-direction buffers and
    // unpacked again. Communication bytes count the serialised payload of both directions.
    {
      ByteView bufLeft(Kokkos::view_alloc(Kokkos::WithoutInitializing, "bench:left"), n * pb);
      ByteView bufRight(Kokkos::view_alloc(Kokkos::WithoutInitializing, "bench:right"), n * pb);
      ByteView scratch(
          Kokkos::view_alloc(Kokkos::WithoutInitializing, "bench:scratch"), Traits::partitionScratchBytes(c)
      );
      std::byte *outs[2] = {bufLeft.data(), bufRight.data()};
      std::size_t counts[2] = {0, 0};
      Sample s;
      for (int r = 0; r < cfg.repeats; ++r) {
        HeapWindow window;
        const double ms = timedMs([&] {
          Traits::countBuckets(c, MigrateClassifier{}, counts, 2);
          Traits::template partitionAndPack<Serializer>(c, MigrateClassifier{}, outs, 2, scratch.data());
          Traits::template bulkInsert<Serializer>(c, bufLeft.data(), counts[0]);
          Traits::template bulkInsert<Serializer>(c, bufRight.data(), counts[1]);
        });
        s.add(ms, window.finish());
      }
      checkSize(c, "migrate");
      const double comm = static_cast<double>((counts[0] + counts[1]) * pb);
      fillContainerDiagnostics(
          emit(
              "migrate", s, comm, "loopback, left=" + std::to_string(counts[0]) + " right=" + std::to_string(counts[1])
          ),
          c
      );
    }

    // 5. Push with interpolation, one sample per step after a warm-up step.
    const int cells = static_cast<int>(cfg.fieldCells);
    {
      FieldView field = [cells] {
        Kokkos::View<double *, Kokkos::HostSpace> host("bench:field-host", cells);
        for (int i = 0; i < cells; ++i) {
          host(i) = 0.1 * std::sin(kTwoPi * static_cast<double>(i) / static_cast<double>(cells));
        }
        FieldView dev(Kokkos::view_alloc(Kokkos::WithoutInitializing, "bench:field"), cells);
        Kokkos::deep_copy(dev, host);
        return dev;
      }();
      PushInterp<FieldView> push{field, cells};
      forEachParticle(c, push);
      Sample s;
      for (int step = 0; step < cfg.steps; ++step) {
        measure(s, [&] { forEachParticle(c, push); });
      }
      checkSize(c, "push_interp");
      fillContainerDiagnostics(emit("push_interp", s, 0.0, "per step; interpolation + update"), c);
    }

    // 6. Cloud-in-cell deposition, one sample per step after a warm-up step. The current array is zeroed untimed.
    {
      FieldView current(Kokkos::view_alloc(Kokkos::WithoutInitializing, "bench:current"), cells);
      DepositCic<FieldView> deposit{current, cells};
      Kokkos::deep_copy(current, 0.0);
      forEachParticle(c, deposit);
      Sample s;
      for (int step = 0; step < cfg.steps; ++step) {
        Kokkos::deep_copy(current, 0.0);
        measure(s, [&] { forEachParticle(c, deposit); });
      }
      checkSize(c, "deposit");
      fillContainerDiagnostics(emit("deposit", s, 0.0, "per step; atomic CIC accumulation"), c);
    }

    // 7. Rebin into spatial bins. Only the blocked host container supports this operation.
    if constexpr (IsBlock<C>::value) {
      auto binOf = [](const Particle &p) {
        const auto b = static_cast<std::size_t>(p.x[0] * static_cast<double>(kBins));
        return std::min(b, kBins - 1);
      };
      Sample s;
      for (int r = 0; r < cfg.repeats; ++r) {
        measure(s, [&] { c.rebin(kBins, binOf); });
      }
      std::size_t maxBin = 0;
      for (std::size_t b = 0; b < c.binCount(); ++b) {
        std::size_t count = 0;
        c.forEachInBin(b, [&](const Particle &) { ++count; });
        maxBin = std::max(maxBin, count);
      }
      const double mean = static_cast<double>(n) / static_cast<double>(kBins);
      checkSize(c, "rebin");
      fillContainerDiagnostics(
          emit("rebin", s, 0.0, "bins=" + std::to_string(kBins) + " max/mean=" + std::to_string(maxBin / mean)), c
      );
    } else {
      emitNa("rebin", "block container only");
    }
  }

  std::string cpuModel() {
    std::ifstream in("/proc/cpuinfo");
    std::string line;
    while (std::getline(in, line)) {
      if (line.rfind("model name", 0) == 0) {
        const auto pos = line.find(':');
        if (pos != std::string::npos) {
          const auto first = line.find_first_not_of(" \t", pos + 1);
          return first == std::string::npos ? std::string("unknown") : line.substr(first);
        }
      }
    }
    return "unknown";
  }

  std::string gpuName() {
#if defined(KOKKOS_ENABLE_CUDA)
    int device = 0;
    cudaDeviceProp prop{};
    if (cudaGetDevice(&device) == cudaSuccess && cudaGetDeviceProperties(&prop, device) == cudaSuccess) {
      return prop.name;
    }
    return "unknown";
#else
    return "none (no GPU backend)";
#endif
  }

  void printEnvironment(const Config &cfg) {
    const int kokkosVersion = KOKKOS_VERSION;
    std::printf("# schnek particle container benchmark (E1)\n");
    std::printf("# git revision      : %s\n", SCHNEK_BENCH_GIT_REVISION);
    std::printf("# build type        : %s\n", SCHNEK_BENCH_BUILD_TYPE);
    std::printf("# compiler          : %s\n", __VERSION__);
    std::printf(
        "# kokkos version    : %d.%d.%d\n", kokkosVersion / 10000, (kokkosVersion / 100) % 100, kokkosVersion % 100
    );
    std::printf(
        "# default exec/mem  : %s / %s\n", Kokkos::DefaultExecutionSpace::name(),
        Kokkos::DefaultExecutionSpace::memory_space::name()
    );
    std::printf("# cpu               : %s\n", cpuModel().c_str());
    std::printf("# gpu               : %s\n", gpuName().c_str());
    std::printf(
        "# workload          : particles=%zu particle_bytes=%zu block_capacity=%zu steps=%d repeats=%d seed=%llu\n",
        cfg.particles, sizeof(Particle), cfg.blockCapacity, cfg.steps, cfg.repeats,
        static_cast<unsigned long long>(cfg.seed)
    );
    std::printf("# profiles          :");
    for (const auto &p : cfg.profiles) {
      std::printf(" %s", p.c_str());
    }
    std::printf("\n# times are per operation (push_interp and deposit: per step); median/min/max over samples\n");
    std::printf("# alloc = host allocations in the last sample; peakKiB = host heap above the window start\n\n");
  }

  std::string cell(std::size_t v) {
    return v == kNotApplicable ? std::string("-") : std::to_string(v);
  }

  void printTable(const std::vector<Record> &records) {
    std::printf(
        "%-10s %-16s %-24s %-13s %10s %10s %10s %8s %10s %10s %10s %10s %8s %8s %8s %10s  %s\n", "profile", "candidate",
        "space", "op", "median_ms", "min_ms", "max_ms", "alloc", "alloc_KiB", "peakKiB", "size", "reservedKiB", "util",
        "reallocs", "blocks", "commKiB", "note"
    );
    for (const auto &r : records) {
      if (!r.supported) {
        std::printf(
            "%-10s %-16s %-24s %-13s %10s %10s %10s %8s %10s %10s %10s %10s %8s %8s %8s %10s  %s\n", r.profile.c_str(),
            r.candidate.c_str(), r.space.c_str(), r.op.c_str(), "n/a", "n/a", "n/a", "-", "-", "-", "-", "-", "-", "-",
            "-", "-", r.note.c_str()
        );
        continue;
      }
      const double util = r.capacity == 0 ? 0.0 : static_cast<double>(r.size) / static_cast<double>(r.capacity);
      std::printf(
          "%-10s %-16s %-24s %-13s %10.4f %10.4f %10.4f %8zu %10.1f %10.1f %10zu %10.1f %8.3f %8s %8s %10.1f  %s\n",
          r.profile.c_str(), r.candidate.c_str(), r.space.c_str(), r.op.c_str(), r.sample.median(), r.sample.minimum(),
          r.sample.maximum(), r.sample.heap.count, r.sample.heap.bytes / 1024.0, r.sample.heap.peakExtra / 1024.0,
          r.size, r.reserved / 1024.0, util, cell(r.reallocs).c_str(), cell(r.blocks).c_str(), r.commBytes / 1024.0,
          r.note.c_str()
      );
    }
  }

  void writeJson(const std::vector<Record> &records, const std::string &path, const Config &cfg) {
    std::ofstream os(path);
    if (!os) {
      throw std::runtime_error("cannot open " + path + " for writing");
    }
    for (const auto &r : records) {
      os << "{\"profile\":\"" << r.profile << "\",\"candidate\":\"" << r.candidate << "\",\"space\":\"" << r.space
         << "\",\"op\":\"" << r.op << "\",\"git\":\"" << SCHNEK_BENCH_GIT_REVISION
         << "\",\"particles\":" << cfg.particles << ",\"block_capacity\":" << cfg.blockCapacity
         << ",\"field_cells\":" << cfg.fieldCells << ",\"seed\":" << cfg.seed;
      if (!r.supported) {
        os << ",\"supported\":false,\"note\":\"" << r.note << "\"}\n";
        continue;
      }
      const double util = r.capacity == 0 ? 0.0 : static_cast<double>(r.size) / static_cast<double>(r.capacity);
      os << ",\"samples\":" << r.sample.ms.size() << ",\"median_ms\":" << r.sample.median()
         << ",\"min_ms\":" << r.sample.minimum() << ",\"max_ms\":" << r.sample.maximum()
         << ",\"alloc_count\":" << r.sample.heap.count << ",\"alloc_bytes\":" << r.sample.heap.bytes
         << ",\"peak_extra_bytes\":" << r.sample.heap.peakExtra << ",\"size\":" << r.size
         << ",\"capacity\":" << r.capacity << ",\"reserved_bytes\":" << r.reserved << ",\"utilisation\":" << util;
      if (r.reallocs != kNotApplicable) {
        os << ",\"reallocations\":" << r.reallocs;
      }
      if (r.blocks != kNotApplicable) {
        os << ",\"blocks\":" << r.blocks;
      }
      os << ",\"comm_bytes\":" << r.commBytes << ",\"note\":\"" << r.note << "\"}\n";
    }
  }

  // C2 binning study. Storage-order push and CIC deposition on the block container, with the bins refreshed by
  // rebin at a given period. The field size comes from --field-cells. A field that fits in cache hides locality
  // effects, so the study should be run with a field well above the cache size.
  struct StudyPolicy {
      std::string name;
      int period;  // rebin every `period` steps; 0 never rebins
  };

  struct StudyRow {
      std::string profile;
      std::size_t bins = 0;
      std::string policy;
      bool binned = false;
      std::vector<double> traversalMs;      // push + deposit, one entry per step and repeat
      std::vector<double> rebinMs;          // one entry per rebin
      double totalPerStepMs = 0.0;          // mean of traversal + rebin per step
      double maxOverMean = std::nan("");    // mean over rebins of the largest bin relative to the mean bin
      double staleFraction = std::nan("");  // mean fraction of particles outside their assigned bin before a push
  };

  std::size_t binIndexX(const Particle &p, std::size_t bins) {
    const auto b = static_cast<std::size_t>(p.x[0] * static_cast<double>(bins));
    return std::min(b, bins - 1);
  }

  double medianOf(const std::vector<double> &values) {
    if (values.empty()) {
      return std::nan("");
    }
    Sample s;
    s.ms = values;
    return s.median();
  }

  StudyRow runStudyCase(
      const std::vector<Particle> &parts,
      const std::string &profile,
      std::size_t bins,
      const StudyPolicy &policy,
      const Config &cfg
  ) {
    using FieldHost = Kokkos::View<double *, Kokkos::HostSpace>;
    const std::size_t n = parts.size();
    const int cells = static_cast<int>(cfg.fieldCells);
    const bool binned = policy.period > 0;

    StudyRow row;
    row.profile = profile;
    row.bins = binned ? bins : 0;
    row.policy = policy.name;
    row.binned = binned;

    FieldHost field("study:field", cfg.fieldCells);
    for (int i = 0; i < cells; ++i) {
      field(i) = 0.1 * std::sin(kTwoPi * static_cast<double>(i) / static_cast<double>(cells));
    }
    FieldHost current("study:current", cfg.fieldCells);
    PushInterp<FieldHost> push{field, cells};
    DepositCic<FieldHost> deposit{current, cells};

    double totalMs = 0.0;
    double occSum = 0.0;
    std::size_t occCount = 0;
    double staleSum = 0.0;
    std::size_t staleCount = 0;

    for (int r = 0; r < cfg.repeats; ++r) {
      HostBlock c(cfg.blockCapacity);
      c.insert(parts.begin(), parts.end());
      for (int step = 0; step < cfg.studySteps; ++step) {
        if (binned && step % policy.period == 0) {
          const double ms = timedMs([&] { c.rebin(bins, [bins](const Particle &p) { return binIndexX(p, bins); }); });
          row.rebinMs.push_back(ms);
          totalMs += ms;

          // Untimed: largest bin relative to the mean bin occupancy.
          std::size_t maxCount = 0;
          for (std::size_t b = 0; b < bins; ++b) {
            std::size_t count = 0;
            c.forEachInBin(b, [&](const Particle &) { ++count; });
            maxCount = std::max(maxCount, count);
          }
          occSum += static_cast<double>(maxCount) * static_cast<double>(bins) / static_cast<double>(n);
          ++occCount;
        }
        if (binned) {
          // Untimed: particles that have moved out of the bin they were assigned at the last rebin.
          std::size_t stale = 0;
          for (std::size_t b = 0; b < bins; ++b) {
            c.forEachInBin(b, [&](const Particle &p) { stale += binIndexX(p, bins) != b ? 1 : 0; });
          }
          staleSum += static_cast<double>(stale) / static_cast<double>(n);
          ++staleCount;
        }

        const double tPush = timedMs([&] { c.forEach([&](Particle &p) { push(p); }); });
        Kokkos::deep_copy(current, 0.0);
        const double tDep = timedMs([&] { c.forEach([&](const Particle &p) { deposit(p); }); });

        // Each particle has unit weight, so the deposited total must equal N if every particle was visited once.
        double weight = 0.0;
        for (int i = 0; i < cells; ++i) {
          weight += current(i);
        }
        if (c.size() != n || std::abs(weight - static_cast<double>(n)) > 1e-6 * static_cast<double>(n)) {
          throw std::runtime_error("binning study: traversal did not visit every particle exactly once");
        }
        row.traversalMs.push_back(tPush + tDep);
        totalMs += tPush + tDep;
      }
    }

    row.totalPerStepMs = totalMs / static_cast<double>(cfg.repeats * cfg.studySteps);
    row.maxOverMean = occCount > 0 ? occSum / static_cast<double>(occCount) : std::nan("");
    row.staleFraction = staleCount > 0 ? staleSum / static_cast<double>(staleCount) : std::nan("");
    return row;
  }

  std::vector<StudyRow> runBinningStudy(const Config &cfg) {
    const std::vector<StudyPolicy> policies = {{"every-step", 1}, {"every-10", 10}, {"every-50", 50}};
    const std::vector<std::size_t> binCounts = {64, 1024};
    std::vector<StudyRow> rows;
    for (const auto &profile : cfg.profiles) {
      const std::vector<Particle> parts = makeParticles(cfg.particles, profile == "clustered", cfg.seed);
      rows.push_back(runStudyCase(parts, profile, 0, StudyPolicy{"none", 0}, cfg));
      for (const auto &policy : policies) {
        for (const std::size_t bins : binCounts) {
          rows.push_back(runStudyCase(parts, profile, bins, policy, cfg));
        }
      }
    }
    return rows;
  }

  std::string formatOrDash(double value, int precision) {
    if (std::isnan(value)) {
      return "-";
    }
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.*f", precision, value);
    return buf;
  }

  void printStudy(const std::vector<StudyRow> &rows, const Config &cfg) {
    std::printf(
        "\n# C2 binning study: %zu particles, field %zu cells, %d steps x %d repeats\n", cfg.particles, cfg.fieldCells,
        cfg.studySteps, cfg.repeats
    );
    std::printf(
        "%-10s %6s %-11s %12s %10s %7s %14s %10s %8s\n", "profile", "bins", "policy", "trav ms/step", "rebin ms",
        "rebins", "total ms/step", "max/mean", "stale"
    );
    for (const auto &row : rows) {
      const std::string bins = row.binned ? std::to_string(row.bins) : "-";
      const std::size_t rebinsPerRun = row.rebinMs.size() / static_cast<std::size_t>(cfg.repeats);
      std::printf(
          "%-10s %6s %-11s %12s %10s %7zu %14s %10s %8s\n", row.profile.c_str(), bins.c_str(), row.policy.c_str(),
          formatOrDash(medianOf(row.traversalMs), 3).c_str(), formatOrDash(medianOf(row.rebinMs), 3).c_str(),
          rebinsPerRun, formatOrDash(row.totalPerStepMs, 3).c_str(), formatOrDash(row.maxOverMean, 2).c_str(),
          formatOrDash(row.staleFraction, 3).c_str()
      );
    }
  }

}  // namespace

int main(int argc, char *argv[]) {
  Config cfg;
  if (!parseArgs(argc, argv, cfg)) {
    return 2;
  }

  Kokkos::initialize(argc, argv);
  int status = 0;
  try {
    printEnvironment(cfg);
    if (!cfg.study.empty()) {
      printStudy(runBinningStudy(cfg), cfg);
      Kokkos::finalize();
      return 0;
    }
    std::vector<Record> records;
    for (const auto &profile : cfg.profiles) {
      const bool clustered = profile == "clustered";
      const std::vector<Particle> parts = makeParticles(cfg.particles, clustered, cfg.seed);

      runCandidate<std::vector<Particle>>("std::vector", profile, parts, cfg, records);
      runCandidate<HostBlock>("block-host", profile, parts, cfg, records);
      runCandidate<DenseHost>("dense", profile, parts, cfg, records);
      runCandidate<DeviceBlockedHost>("device-blocked", profile, parts, cfg, records);
      runCandidate<DenseDefault>("dense", profile, parts, cfg, records);
      runCandidate<DeviceBlockedDefault>("device-blocked", profile, parts, cfg, records);
    }
    printTable(records);
    if (!cfg.jsonPath.empty()) {
      writeJson(records, cfg.jsonPath, cfg);
    }
  } catch (const std::exception &e) {
    std::fprintf(stderr, "error: %s\n", e.what());
    status = 1;
  }
  Kokkos::finalize();
  return status;
}
