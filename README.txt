Title  : Text Editor using ncurses.
Name   : Prasad Mukund Pawar
MIS ID : 111603051

Description:

This editor has a buffer made of gap data structure, only reason of choosing the gap data structure was that it made insertion and deletion easier compared to other data structures, although making some operations quite difficult which I realised later(PgeUp/PgeDown). 
It supports the functions like movement of cursor(Up, Down, Left, Right) 
and inserting at the cursor position including Backspace, Space, Tab, Enter. 
Other features are - Cut/Copy/Paste a line. 
Find - searches the word from the current cursor position, doesnot search multiple occurences in the file. Replace - replaces with the given word from the current cursor position.
Saving a file. However it does not support a file which has no of lines more than the screen size.
