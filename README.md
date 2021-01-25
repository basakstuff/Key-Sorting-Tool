# Başak ER 
## KeySorting Indexing Tool

● open command<br />
This command takes a parameter that is the name of the json file. It opens the json file and
reads the given settings. Then, it reads the whole index file in to memory and waits for the
further commands.<br />
● create_index command<br />
This command creates the index file using key field which is specified in the json file. You
need to implement the simple indexing method and you should use the relative record number
or absolute byte address as pointer. You can use any sorting algorithm you wish.<br />
● search command<br />
This command finds the record for the given key in index file using binary search and fetches
corresponding record from datafile.<br />
● close command<br />
This command closes the used files and frees the memory.<br />
