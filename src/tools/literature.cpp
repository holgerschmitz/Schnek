/*
 * literature.cpp
 *
 * Created on: 13 Nov 2012
 * Author: Holger Schmitz
 * Email: holger@notjustphysics.com
 *
 * Copyright 2012 Holger Schmitz
 *
 * This file is part of OPar.
 *
 * OPar is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OPar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OPar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "literature.hpp"

#include <boost/foreach.hpp>

#include <iostream>

using namespace schnek;

std::string LiteratureReference::getPublicationTypeString() const
{
  switch (publType)
  {
    case article:
      return "article";
      break;
    case book:
      return "book";
      break;
    case booklet:
      return "booklet";
      break;
    case inbook:
      return "inbook";
      break;
    case incollection:
      return "incollection";
      break;
    case inproceedings:
      return "inproceedings";
      break;
    case manual:
      return "manual";
      break;
    case mastersthesis:
      return "mastersthesis";
      break;
    case misc:
      return "misc";
      break;
    case phdthesis:
      return "phdthesis";
      break;
    case proceedings:
      return "proceedings";
      break;
    case techreport:
      return "techreport";
      break;
    default:
    case unpublished:
      return "unpublished";
      break;
  }
}

void writeRefField(std::ostream &out, std::string name, std::string value)
{
  if (value != "") out << "  " << name << " = {" << value << "}\n";
}

std::ostream &operator<<(std::ostream &out, const LiteratureReference &lit)
{
  out << "@" << lit.getPublicationTypeString() << " {" << lit.getBibKey()
      << "\n";
  writeRefField(out, "address", lit.getAddress());
  writeRefField(out, "annote", lit.getAnnote());
  writeRefField(out, "author", lit.getAuthor());
  writeRefField(out, "booktitle", lit.getBooktitle());
  writeRefField(out, "chapter", lit.getChapter());
  writeRefField(out, "crossref", lit.getCrossref());
  writeRefField(out, "edition", lit.getEdition());
  writeRefField(out, "editor", lit.getEditor());
  writeRefField(out, "eprint", lit.getEprint());
  writeRefField(out, "howpublished", lit.getHowpublished());
  writeRefField(out, "institution", lit.getInstitution());
  writeRefField(out, "journal", lit.getJournal());
  writeRefField(out, "key", lit.getKey());
  writeRefField(out, "month", lit.getMonth());
  writeRefField(out, "note", lit.getNote());
  writeRefField(out, "number", lit.getNumber());
  writeRefField(out, "organization", lit.getOrganization());
  writeRefField(out, "pages", lit.getPages());
  writeRefField(out, "publisher", lit.getPublisher());
  writeRefField(out, "school", lit.getSchool());
  writeRefField(out, "series", lit.getSeries());
  writeRefField(out, "title", lit.getTitle());
  writeRefField(out, "type", lit.getType());
  writeRefField(out, "url", lit.getUrl());
  writeRefField(out, "volume", lit.getVolume());
  writeRefField(out, "year", lit.getYear());
  out << "}\n\n";

  return out;
}

LiteratureManager::LiteratureManager()
  : title("Algorithms and Methods"),
    subtitle("The following algorithms and methods have been used during this simulation run.")
{}

void LiteratureManager::addReference(std::string description,
    const LiteratureReference &reference)
{
  std::string bibKey = reference.getBibKey();
  if (records.count(bibKey) > 0)
  {
    records[bibKey].first.insert(description);
  }
  else
  {
    LitRecord rec;
    rec.first.insert(description);
    rec.second = reference;
    records.insert(Records::value_type(bibKey, rec));
  }
}

void LiteratureManager::writeInformation(std::ostream &out, std::string bibfile)
{
  out << "\\documentclass{article}\n" << "\\begin{document}\n"
      << "\\section*{"<< title <<"}\n"
      << subtitle << "\n\\begin{itemize}";
  BOOST_FOREACH(Records::value_type rec, records)
  {
    out << "\\item ";
    BOOST_FOREACH(std::string desc, rec.second.first)
      out << desc << "\n";
    out << "\\cite{" << rec.first << "}\n";
  }
  out << "\\end{itemize}\n" << "\\bibliographystyle{acm}\n" << "\\bibliography{"
      << bibfile << "}\n" << "\\end{document}\n";
}

void LiteratureManager::writeBibTex(std::ostream &out)
{
  BOOST_FOREACH(Records::value_type rec, records)
  {
    out << rec.second.second << std::endl;
  }
}

std::ostream& schnek::operator<<(std::ostream &out,
    const schnek::LiteratureReference &ref)
{
  out << "@" << ref.getPublicationTypeString() << "{" << ref.getBibKey();
  if (ref.getAuthor() != "") out << ",\n  author={" << ref.getAuthor() << "}";
  if (ref.getTitle() != "") out << ",\n  title={" << ref.getTitle() << "}";
  if (ref.getJournal() != "") out << ",\n  journal={" << ref.getJournal()
      << "}";
  if (ref.getBooktitle() != "") out << ",\n  booktitle={" << ref.getBooktitle()
      << "}";
  if (ref.getEdition() != "") out << ",\n  edition={" << ref.getEdition()
      << "}";
  if (ref.getVolume() != "") out << ",\n  volume={" << ref.getVolume() << "}";
  if (ref.getChapter() != "") out << ",\n  chapter={" << ref.getChapter()
      << "}";
  if (ref.getNumber() != "") out << ",\n  number={" << ref.getNumber() << "}";
  if (ref.getMonth() != "") out << ",\n  month={" << ref.getMonth() << "}";
  if (ref.getYear() != "") out << ",\n  year={" << ref.getYear() << "}";
  if (ref.getPages() != "") out << ",\n  pages={" << ref.getPages() << "}";
  if (ref.getEditor() != "") out << ",\n  editor={" << ref.getEditor() << "}";
  if (ref.getSeries() != "") out << ",\n  series={" << ref.getSeries() << "}";
  if (ref.getInstitution() != "") out << ",\n  institution={"
      << ref.getInstitution() << "}";
  if (ref.getOrganization() != "") out << ",\n  organization={"
      << ref.getOrganization() << "}";
  if (ref.getSchool() != "") out << ",\n  school={" << ref.getSchool() << "}";
  if (ref.getPublisher() != "") out << ",\n  publisher={" << ref.getPublisher()
      << "}";
  if (ref.getAddress() != "") out << ",\n  address={" << ref.getAddress()
      << "}";

  if (ref.getAnnote() != "") out << ",\n  annote={" << ref.getAnnote() << "}";
  if (ref.getCrossref() != "") out << ",\n  crossref={" << ref.getCrossref()
      << "}";
  if (ref.getEprint() != "") out << ",\n  eprint={" << ref.getEprint() << "}";
  if (ref.getHowpublished() != "") out << ",\n  howpublished={"
      << ref.getHowpublished() << "}";

  if (ref.getKey() != "") out << ",\n  key={" << ref.getKey() << "}";
  if (ref.getNote() != "") out << ",\n  note={" << ref.getNote() << "}";
  if (ref.getType() != "") out << ",\n  type={" << ref.getType() << "}";
  if (ref.getUrl() != "") out << ",\n  url={" << ref.getUrl() << "}";

  out << "\n}\n\n";
  return out;

}
