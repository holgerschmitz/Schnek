/*
 * literature.hpp
 *
 * Created on: 13 Nov 2012
 * Author: Holger Schmitz
 * Email: holger@notjustphysics.com
 *
 * Copyright 2012 Holger Schmitz
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

#ifndef SCHNEK_LITERATURE_HPP_
#define SCHNEK_LITERATURE_HPP_

#include <boost/tuple/tuple.hpp>

namespace schnek {

class LiteratureReference
{
  public:
      typedef enum {article, book, booklet, inbook, incollection,
        inproceedings, manual, mastersthesis, misc, phdthesis,
        proceedings, techreport, unpublished } PublicationType;
private:
    // All the information needed for a BibTex entry.
    // Descriptions of the fields are taken from Wikipedia
    std::string address; //  Publisher's address (usually just the city, but can be the full address for lesser-known publishers)
    std::string annote; // An annotation for annotated bibliography styles (not typical)
    std::string author; // The name(s) of the author(s) (in the case of more than one author, separated by and)
    std::string booktitle; // The title of the book, if only part of it is being cited
    std::string chapter; // The chapter number
    std::string crossref; // The key of the cross-referenced entry
    std::string edition; // The edition of a book, long form (such as "first" or "second")
    std::string editor; // The name(s) of the editor(s)
    std::string eprint; // A specification of an electronic publication, often a preprint or a technical report
    std::string howpublished; // How it was published, if the publishing method is nonstandard
    std::string institution; // The institution that was involved in the publishing, but not necessarily the publisher
    std::string journal; // The journal or magazine the work was published in
    std::string key; // A hidden field used for specifying or overriding the alphabetical order of entries (when the "author" and "editor" fields are missing). Note that this is very different from the key (mentioned just after this list) that is used to cite or cross-reference the entry.
    std::string month; // The month of publication (or, if unpublished, the month of creation)
    std::string note; // Miscellaneous extra information
    std::string number; // The "(issue) number" of a journal, magazine, or tech-report, if applicable. (Most publications have a "volume", but no "number" field.)
    std::string organization; // The conference sponsor
    std::string pages; // Page numbers, separated either by commas or double-hyphens.
    std::string publisher; // The publisher's name
    std::string school; // The school where the thesis was written
    std::string series; // The series of books the book was published in (e.g. "The Hardy Boys" or "Lecture Notes in Computer Science")
    std::string title; // The title of the work
    std::string type; // The field overriding the default type of publication (e.g. "Research Note" for techreport, "{PhD} dissertation" for phdthesis, "Section" for inbook/incollection)
    std::string url; // The WWW address
    std::string volume; // The volume of a journal or multi-volume book
    std::string year; // The year of publication (or, if unpublished, the year of creation)

    std::string bibKey;
    PublicationType publType;
  public:
    LiteratureReference() {}
    LiteratureReference(std::string bibKey_) : bibKey(bibKey_) {}

    LiteratureReference(const LiteratureReference &ref) :
      address(ref.address),
      annote(ref.annote),
      author(ref.author),
      booktitle(ref.booktitle),
      chapter(ref.chapter),
      crossref(ref.crossref),
      edition(ref.edition),
      editor(ref.editor),
      eprint(ref.eprint),
      howpublished(ref.howpublished),
      institution(ref.institution),
      journal(ref.journal),
      key(ref.key),
      month(ref.month),
      note(ref.note),
      number(ref.number),
      organization(ref.organization),
      pages(ref.pages),
      publisher(ref.publisher),
      school(ref.school),
      series(ref.series),
      title(ref.title),
      type(ref.type),
      url(ref.url),
      volume(ref.volume),
      year(ref.year),
      bibKey(ref.bibKey)
    {}

    LiteratureReference &operator=(const LiteratureReference &ref)
    {
      address = ref.address;
      annote = ref.annote;
      author = ref.author;
      booktitle = ref.booktitle;
      chapter = ref.chapter;
      crossref = ref.crossref;
      edition = ref.edition;
      editor = ref.editor;
      eprint = ref.eprint;
      howpublished = ref.howpublished;
      institution = ref.institution;
      journal = ref.journal;
      key = ref.key;
      month = ref.month;
      note = ref.note;
      number = ref.number;
      organization = ref.organization;
      pages = ref.pages;
      publisher = ref.publisher;
      school = ref.school;
      series = ref.series;
      title = ref.title;
      type = ref.type;
      url = ref.url;
      volume = ref.volume;
      year = ref.year;
      bibKey = ref.bibKey;

      return *this;
    }

    std::string getPublicationTypeString() const;

    PublicationType getPublicationType() const
    {
        return publType;
    }

    std::string getAddress() const
    {
        return address;
    }

    std::string getAnnote() const
    {
        return annote;
    }

    std::string getAuthor() const
    {
        return author;
    }

    std::string getBooktitle() const
    {
        return booktitle;
    }

    std::string getChapter() const
    {
        return chapter;
    }

    std::string getCrossref() const
    {
        return crossref;
    }

    std::string getEdition() const
    {
        return edition;
    }

    std::string getEditor() const
    {
        return editor;
    }

    std::string getEprint() const
    {
        return eprint;
    }

    std::string getHowpublished() const
    {
        return howpublished;
    }

    std::string getInstitution() const
    {
        return institution;
    }

    std::string getJournal() const
    {
        return journal;
    }

    std::string getKey() const
    {
        return key;
    }

    std::string getMonth() const
    {
        return month;
    }

    std::string getNote() const
    {
        return note;
    }

    std::string getNumber() const
    {
        return number;
    }

    std::string getOrganization() const
    {
        return organization;
    }

    std::string getPages() const
    {
        return pages;
    }

    std::string getPublisher() const
    {
        return publisher;
    }

    std::string getSchool() const
    {
        return school;
    }

    std::string getSeries() const
    {
        return series;
    }

    std::string getTitle() const
    {
        return title;
    }

    std::string getType() const
    {
        return type;
    }

    std::string getUrl() const
    {
        return url;
    }

    std::string getVolume() const
    {
        return volume;
    }

    std::string getYear() const
    {
        return year;
    }

    virtual std::string getBibKey() const
    {
      return bibKey;
    }

    void setPublicationType(PublicationType publType)
    {
        this->publType = publType;
    }

    void setAddress(std::string address)
    {
        this->address = address;
    }

    void setAnnote(std::string annote)
    {
        this->annote = annote;
    }

    void setAuthor(std::string author)
    {
        this->author = author;
    }

    void setBooktitle(std::string booktitle)
    {
        this->booktitle = booktitle;
    }

    void setChapter(std::string chapter)
    {
        this->chapter = chapter;
    }

    void setCrossref(std::string crossref)
    {
        this->crossref = crossref;
    }

    void setEdition(std::string edition)
    {
        this->edition = edition;
    }

    void setEditor(std::string editor)
    {
        this->editor = editor;
    }

    void setEprint(std::string eprint)
    {
        this->eprint = eprint;
    }

    void setHowpublished(std::string howpublished)
    {
        this->howpublished = howpublished;
    }

    void setInstitution(std::string institution)
    {
        this->institution = institution;
    }

    void setJournal(std::string journal)
    {
        this->journal = journal;
    }

    void setKey(std::string key)
    {
        this->key = key;
    }

    void setMonth(std::string month)
    {
        this->month = month;
    }

    void setNote(std::string note)
    {
        this->note = note;
    }

    void setNumber(std::string number)
    {
        this->number = number;
    }

    void setOrganization(std::string organization)
    {
        this->organization = organization;
    }

    void setPages(std::string pages)
    {
        this->pages = pages;
    }

    void setPublisher(std::string publisher)
    {
        this->publisher = publisher;
    }

    void setSchool(std::string school)
    {
        this->school = school;
    }

    void setSeries(std::string series)
    {
        this->series = series;
    }

    void setTitle(std::string title)
    {
        this->title = title;
    }

    void setType(std::string type)
    {
        this->type = type;
    }

    void setUrl(std::string url)
    {
        this->url = url;
    }

    void setVolume(std::string volume)
    {
        this->volume = volume;
    }

    void setYear(std::string year)
    {
        this->year = year;
    }

};


class LiteratureArticle : public LiteratureReference
{
  public:
    LiteratureArticle(std::string bibKey, std::string author, std::string title, std::string journal, std::string year,
        std::string volume = "", std::string number = "", std::string pages = "", std::string month = "",
        std::string note = "", std::string key = "")
      : LiteratureReference(bibKey)
    {
      setPublicationType(article);

      setAuthor(author);
      setTitle(title);
      setJournal(journal);
      setYear(year);
      setVolume(volume);
      setNumber(number);
      setPages(pages);
      setMonth(month);
      setNote(note);
      setKey(key);

    }
};

class LiteratureBook : public LiteratureReference
{
  public:
    LiteratureBook(std::string bibKey, std::string author, std::string editor, std::string title, std::string publisher, std::string year,
        std::string volume = "", std::string number = "", std::string series = "", std::string address = "",
        std::string edition = "", std::string month = "", std::string note = "", std::string key = "")
      : LiteratureReference(bibKey)
    {
      setPublicationType(book);

      setAuthor(author);
      setEditor(editor);
      setTitle(title);
      setPublisher(publisher);
      setYear(year);
      setVolume(volume);
      setNumber(number);
      setSeries(series);
      setAddress(address);
      setEdition(edition);
      setMonth(month);
      setNote(note);
      setKey(key);
    }
};


class LiteratureBooklet : public LiteratureReference
{
  public:
    LiteratureBooklet(std::string bibKey, std::string title,
        std::string author = "", std::string howpublished = "", std::string address = "",
        std::string month = "", std::string year= "", std::string note = "", std::string key = "")
      : LiteratureReference(bibKey)
    {
      setPublicationType(booklet);

      setTitle(title);
      setAuthor(author);
      setHowpublished(howpublished);
      setAddress(address);
      setMonth(month);
      setYear(year);
      setNote(note);
      setKey(key);
    }
};


class LiteratureInBook : public LiteratureReference
{
  public:
    LiteratureInBook(std::string bibKey, std::string author, std::string editor, std::string title, std::string chapter,
        std::string pages, std::string publisher, std::string year,
        std::string volume = "", std::string number = "", std::string series = "", std::string type = "",
        std::string address = "", std::string edition = "", std::string month = "", std::string note = "",
        std::string key = "")
      : LiteratureReference(bibKey)
    {
      setPublicationType(inbook);

      setAuthor(author);
      setEditor(editor);
      setTitle(title);
      setChapter(chapter);
      setPages(pages);
      setPublisher(publisher);
      setYear(year);
      setVolume(volume);
      setNumber(number);
      setSeries(series);
      setType(type);
      setAddress(address);
      setEdition(edition);
      setMonth(month);
      setNote(note);
      setKey(key);
    }
};

class LiteratureInCollection : public LiteratureReference
{
  public:
    LiteratureInCollection(std::string bibKey, std::string author, std::string title, std::string booktitle,
        std::string publisher, std::string year,
        std::string editor = "", std::string volume = "", std::string number = "", std::string series = "",
        std::string type = "", std::string chapter = "", std::string pages = "", std::string address = "",
        std::string edition = "", std::string month = "", std::string note = "", std::string key = "")
      : LiteratureReference(bibKey)
    {
      setPublicationType(incollection);

      setAuthor(author);
      setTitle(title);
      setBooktitle(booktitle);
      setPublisher(publisher);
      setYear(year);
      setEditor(editor);
      setVolume(volume);
      setNumber(number);
      setSeries(series);
      setType(type);
      setChapter(chapter);
      setPages(pages);
      setAddress(address);
      setEdition(edition);
      setMonth(month);
      setNote(note);
      setKey(key);
    }
};


class LiteratureInProceedings : public LiteratureReference
{
  public:
    LiteratureInProceedings(std::string bibKey, std::string author, std::string title, std::string booktitle, std::string year,
        std::string editor = "", std::string volume = "", std::string number = "", std::string series = "",
        std::string pages = "", std::string address = "", std::string month = "", std::string organization = "",
        std::string publisher = "", std::string note = "", std::string key = "")
      : LiteratureReference(bibKey)
    {
      setPublicationType(inproceedings);

      setAuthor(author);
      setTitle(title);
      setBooktitle(booktitle);
      setYear(year);
      setEditor(editor);
      setVolume(volume);
      setNumber(number);
      setSeries(series);
      setPages(pages);
      setAddress(address);
      setMonth(month);
      setEdition(organization);
      setPublisher(publisher);
      setNote(note);
      setKey(key);
    }
};


class LiteratureManual : public LiteratureReference
{
  public:
    LiteratureManual(std::string bibKey, std::string title,
        std::string author = "", std::string organization = "", std::string address = "", std::string edition = "",
        std::string year = "", std::string publisher = "", std::string note = "", std::string key = "")
      : LiteratureReference(bibKey)
    {
      setPublicationType(manual);

      setTitle(title);
      setAuthor(author);
      setEdition(organization);
      setAddress(address);
      setEdition(edition);
      setYear(year);
      setPublisher(publisher);
      setNote(note);
      setKey(key);
    }
};

class LiteratureMastersThesis : public LiteratureReference
{
  public:
    LiteratureMastersThesis(std::string bibKey, std::string author, std::string title, std::string school, std::string year,
        std::string type = "", std::string address = "", std::string month = "",
        std::string note = "", std::string key = "")
      : LiteratureReference(bibKey)
    {
      setPublicationType(mastersthesis);

      setAuthor(author);
      setTitle(title);
      setEdition(school);
      setYear(year);
      setType(type);
      setAddress(address);
      setMonth(month);
      setNote(note);
      setKey(key);
    }
};



class LiteratureMisc : public LiteratureReference
{
  public:
    LiteratureMisc(std::string bibKey, std::string author, std::string title, std::string howpublished = "",
        std::string month = "", std::string year= "", std::string note = "", std::string key = "")
      : LiteratureReference(bibKey)
    {
      setPublicationType(misc);

      setAuthor(author);
      setTitle(title);
      setHowpublished(howpublished);
      setMonth(month);
      setYear(year);
      setNote(note);
      setKey(key);
    }
};


class LiteraturePhdThesis : public LiteratureReference
{
  public:
    LiteraturePhdThesis(std::string bibKey, std::string author, std::string title, std::string school, std::string year,
        std::string type = "", std::string address = "", std::string month = "",
        std::string note = "", std::string key = "")
      : LiteratureReference(bibKey)
    {
      setPublicationType(phdthesis);

      setAuthor(author);
      setTitle(title);
      setEdition(school);
      setYear(year);
      setType(type);
      setAddress(address);
      setMonth(month);
      setNote(note);
      setKey(key);
    }
};


class LiteratureProceedings : public LiteratureReference
{
  public:
    LiteratureProceedings(std::string bibKey, std::string title, std::string year,
        std::string editor = "", std::string volume = "", std::string number = "", std::string series = "",
        std::string address = "", std::string month = "", std::string organization = "",
        std::string publisher = "", std::string note = "", std::string key = "")
      : LiteratureReference(bibKey)
    {
      setPublicationType(proceedings);

      setTitle(title);
      setYear(year);
      setEditor(editor);
      setVolume(volume);
      setNumber(number);
      setSeries(series);
      setAddress(address);
      setMonth(month);
      setEdition(organization);
      setPublisher(publisher);
      setNote(note);
      setKey(key);
    }
};

class LiteratureTechReport : public LiteratureReference
{
  public:
    LiteratureTechReport(std::string bibKey, std::string author, std::string title, std::string institution, std::string year,
        std::string type = "", std::string number = "", std::string address = "", std::string month = "",
        std::string note = "", std::string key = "")
      : LiteratureReference(bibKey)
    {
      setPublicationType(techreport);

      setAuthor(author);
      setTitle(title);
      setInstitution(institution);
      setYear(year);
      setType(type);
      setNumber(number);
      setAddress(address);
      setMonth(month);
      setNote(note);
      setKey(key);
    }
};

class LiteratureUnpublished : public LiteratureReference
{
  public:
    LiteratureUnpublished(std::string bibKey, std::string author, std::string title, std::string note, std::string year,
        std::string month = "", std::string year = "", std::string key = "")
      : LiteratureReference(bibKey)
    {
      setPublicationType(unpublished);

      setAuthor(author);
      setTitle(title);
      setNote(note);
      setMonth(month);
      setYear(year);
      setKey(key);
    }
};


class std::ostream;
class LiteratureManager : public Singleton<LiteratureManager>
{
  private:
    typedef std::list<std::string> Descriptions;
    typedef std::pair<Descriptions, LiteratureReference> LitRecord;
    typedef std::map<std::string, LitRecord> Records;
    Records records;

  public:
    void addReference(std::string description, const LiteratureReference &reference);
    void writeInformation(std::ostream);
    void writeBibTex(std::ostream);

};

std::ostream& operator<<(std::ostream&, const LiteraturReference &);

} // namespace schnek

#endif /* SCHNEK_LITERATURE_HPP_ */
