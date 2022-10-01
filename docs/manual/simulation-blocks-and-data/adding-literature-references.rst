One of the neat features of Schnek is the ability to add literature
references. Many developers in the scientific community may know this
dilemma. You have spent some time and research effort developing a new
numerical scheme. You think that your scheme would greatly improve the
existing simulation codes and should definitely be added into them.
However, you don't want to give away all your work and not be rewarded
for it. After all you have written a publication and you want people to
know about the details of the scheme, why it is better than all the
others, and where it can be applied. Ideally you want users of your new
scheme to cite your publication. If you give your code away by
integrating it in some big simulation code, chances are that it is going
to end up one option amongst many. If you are lucky your literature
reference will be in the manual but most users of the code will probably
not read the manual in all its detail. But fear not, Schnek has a
solution to this dilemma. It allows literature references to be added to
the code. In a well structured simulation code, each numerical scheme
will be contained within one or more modules. In Schnek these modules
are called blocks and are implemented by the ``Block`` class. As a
contributer to a large simulation code you can write a new ``Block``
class that will include its own initialisation routines. Within these
initialisation routines you can add a literature reference. In the main
part of the simulation code, Schnek provides a means to write out all
the literature references of the blocks that have been initialised
during the current execution of the program. This means that a document
will be generated that informs the user which numerical schemes have
been used in a particuler simulation run, together with the literature
references. This makes it easy for any user to cite the right
publications when publishing the simulation results. The base class for
literature references is ``LiteratureReference``. There are a number of
implementations of this class for various types of publications. These
correspond to the `classifications used by
BibTeX <https://en.wikipedia.org/wiki/BibTeX#Entry_types>`__. Each
specific implementation has a constructor that takes a number of strings
as arguments.

::

    LiteratureArticle(string bibKey, string author, string title, string journal, string year,
        string volume = "", string pages = "", string number = "", string month = "",
        string note = "", string key = "")
    LiteratureBook(string bibKey, string author, string editor, string title, string publisher, 
        string year, string volume = "", string number = "", string series = "", string address = "",
        string edition = "", string month = "", string note = "", string key = "")
    LiteratureBooklet(string bibKey, string title,
        string author = "", string howpublished = "", string address = "",
        string month = "", string year= "", string note = "", string key = "")
    LiteratureInBook(string bibKey, string author, string editor, string title, string chapter,
        string pages, string publisher, string year,
        string volume = "", string number = "", string series = "", string type = "",
        string address = "", string edition = "", string month = "", string note = "",
        string key = "")
    LiteratureInCollection(string bibKey, string author, string title, string booktitle,
        string publisher, string year,
        string editor = "", string volume = "", string number = "", string series = "",
        string type = "", string chapter = "", string pages = "", string address = "",
        string edition = "", string month = "", string note = "", string key = "")
    LiteratureInProceedings(string bibKey, string author, string title, string booktitle, string year,
        string editor = "", string volume = "", string number = "", string series = "",
        string pages = "", string address = "", string month = "", string organization = "",
        string publisher = "", string note = "", string key = "")
    LiteratureProceedings(string bibKey, string title, string year,
        string editor = "", string volume = "", string number = "", string series = "",
        string address = "", string month = "", string organization = "",
        string publisher = "", string note = "", string key = "")
    LiteraturePhdThesis(string bibKey, string author, string title, string school, string year,
        string type = "", string address = "", string month = "",
        string note = "", string key = "")
    LiteratureMastersThesis(string bibKey, string author, string title, string school, string year,
        string type = "", string address = "", string month = "",
        string note = "", string key = "")
    LiteratureManual(string bibKey, string title,
        string author = "", string organization = "", string address = "", string edition = "",
        string year = "", string publisher = "", string note = "", string key = "")
    LiteratureTechReport(string bibKey, string author, string title, string institution, string year,
        string type = "", string number = "", string address = "", string month = "",
        string note = "", string key = "")
    LiteratureMisc(string bibKey, string author, string title, string howpublished = "",
        string month = "", string year= "", string note = "", string key = "")
    LiteratureUnpublished(string bibKey, string author, string title, string note, string year,
        string month = "", string key = "")

The ``LiteratureManager`` is then used to add the reference together
with a short description of the usage withing the code. For example, to
add a reference to the Yee algorithm [K. Yee, Antennas Propagation, IEEE
Trans. AP-14, 302 (1966)] for solving Maxwell's equations on a staggered
grid, one could write the following.

::

    LiteratureArticle Yee1966("Yee1966", "Yee, K",
        "Numerical solution of initial boundary value problems involving Maxwell's equations in isotropic media.",
        "IEEE Transactions on Antennas and Propagation", "1966", "AP-14", "302--307");

    LiteratureManager::instance().addReference(
        "Integration of electrodynamic fields uses the Finite Difference Time Domain method.",
        Yee1966);

The ``LiteratureManager`` class is a singleton. This means that access
to the only existing object of this class is through the call to
``LiteratureManager::instance()``. This piece of code would typically go
in the ``init()`` function of the ``Block`` class that performs the
numerical calculations. Once the simulation has been initialised from a
setup file, all relevant literature references will then have been
added. The literature manager provides two functions to write out the
references. The function ``writeBibTex()`` will write a BibTeX file and
``writeInformation()`` will write a LaTeX file that includes all the
short descriptions together with the citations. The following lines of
code would typically go into the main part of the simulation, just after
the call to ``Block::initAll()`` but before the actual calculations
start.

::

    std::ofstream bibTex("references.bib");
    LiteratureManager::instance().writeBibTex(bibTex);
    bibTex.close();

    std::ofstream readme("readme.tex");
    LiteratureManager::instance().writeInformation(readme, "references");
    readme.close();

The code above will write two files, ``references.bib`` and
``readme.tex``. Both calls to ``writeBibTex()`` and
``writeInformation()`` take an output stream as first argument. The call
to ``writeInformation()`` takes the name of the bibliography reference
as second argument. This argument will be translated into the
``\bibliography{references}`` in the LaTeX code. The code for this
example can be found
`here <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_literature.cpp>`__.
