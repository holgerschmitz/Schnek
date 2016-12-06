/*
 * example_array.cpp
 *
 *  Created on: 14 Jul 2014
 *      Author: Holger Schmitz
 *       Email: holger@notjustphysics.com
 */

#include "../src/tools/literature.hpp"

#include <fstream>

using namespace schnek;

int main()
{
  LiteratureArticle Yee1966("Yee1966", "Yee, K",
      "Numerical solution of initial boundary value problems involving Maxwell's equations in isotropic media.",
      "IEEE Transactions on Antennas and Propagation", "1966", "AP-14", "302--307");

  LiteratureManager::instance().addReference(
      "Integration of electrodynamic fields uses the Finite Difference Time Domain method.",
      Yee1966);

  std::ofstream bibTex("references.bib");
  LiteratureManager::instance().writeBibTex(bibTex);
  bibTex.close();

  std::ofstream readme("readme.tex");
  LiteratureManager::instance().writeInformation(readme, "references");
  readme.close();

}
