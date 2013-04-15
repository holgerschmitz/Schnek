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
  if (value != "")
    out << "  " << name << " = {" << value << "}\n";
}

std::ostream &operator<<(std::ostream &out, const LiteraturReference &lit)
{
  out << "@" << lit.getPublicationTypeString() << " {"<< lit.getBibKey() <<"\n";
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

void
LiteratureManager::addReference(std::string description,
    const LiteratureReference &reference)
{
  std::string bibKey = reference.getBibKey();
  if (records.count(bibKey) > 0)
  {
    records[bibKey].first.push_back(description);
  }
  else
  {
    LitRecord rec;
    rec.first.push_back(description);
    rec.second = reference;
    records.insert(Records::value_type(bibKey, rec));
  }
}

void
LiteratureManager::writeInformation(std::ostream out, std::string bibfile)
{
  out << "\\documentclass{article}\n" << "\\begin{document}"
      << "The following alrgorithms have been used\n" << "\\begin{itemize}";
  BOOST_FOREACH(Records::value_type rec, records)
  {
    out << "\\item";
    BOOST_FOREACH(std::string desc, rec.second.first)
    {
      out << desc << "\n";
    }
    out << "\\cite{" << rec.first << "}\n";
  }
  out << "\\bibliography{" << bibfile << "}\n" << "\\end{itemize}\n"
      << "\\end{document}\n";
}

void
LiteratureManager::writeBibTex(std::ostream out)
{
  BOOST_FOREACH(Records::value_type rec, records)
  {
    out << rec.second.second << std::endl;
  }
}
