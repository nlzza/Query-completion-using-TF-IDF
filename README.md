# Query-completion-using-TF-IDF

This program is an intelligent auto completion service which helps a programmer in autocompleting code snippets.
While the programmer is still typing, the pilot calculates what the user is trying to type and suggests a set of most relevant auto completions.

To accomplish this, we acquired 1 to 2 GB of dataset from https://zenodo.org/record/3472050#.YbNU1b1Bzcd
consisting of code snippets of projects from different GitHub repositories.
We create an index of words from our dataset, consisting of a dictionary and a posting list for each word.

The dictionary is a hash table. The first hash table stores all of the first characters.
Each node points to a subsequent hash table which stores the next character.
The last node (which stores the last character of a word) points to a posting list.

The posting list stores the document count (number of documents in which the word appeared),
total count (total number of times the word appeared in the dataset), and a linked list of _Documents_ (a struct).
Each _Document_ stores document ID, term frequency (number of times the term appeared in the document),
and a linked list of line numbers on which the term appeared.

Once the index is ready, we provide the program a partial string, and the program suggests the complete string based on the data in the indexer.
Only the top five suggestions are displayed. The TF-IDF scores for these 5 words are then calculated for every single document that they occur in,
producing a list of highest scoring documents. We then look at the occurrence of the term within these top-scoring documents 
and display the concurrent lines in order to provide relevant suggestions for the line being typed by the user.
