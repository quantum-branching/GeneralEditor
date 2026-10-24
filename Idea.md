The Core Data Structure: I will probably doing a linked list of lines which ends being a linked list of tokens (spaces in raw text files, but allows other delimiters in coding languages)

Autocomplete Algorithm: I will probably do some kind of file analysis which would quickly compile a list of accessible filenames as well a general function name analysis (which will be based on the syntax category of the language, not necessarily the specific language) so that it can finish off file paths, function names, and variable names (possibly and it would prob only be variables in the working file)

Documentation Comments: Comments/documentation about a given token may be given when a user activates it (such as ^D)

Screen Redrawing: I will probably write this in C, writing to stdout whenever there is a change in the working file (partial screen redraw) or a scroll (probably full redraw)
Input: I will probably be using read loop coupled with <termios.h> rather than an asynchronous event loop.
