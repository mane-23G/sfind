# sfind
This is a function that find all files satisfying given test in given directory hierarchies

### Build with
gcc -o sfind sfind.c

### Usage 
sfind \[dir1 dir2 ...\] \[test \]

The user can supply directories, if none are given then the current directory is used. There are two tests that can be used:

-m fileglob : The file being examined satisfies this test if its filename, meaning the pathname stripped of all leading directories, matches the fileglob.

-s filename : The file being examined satisfies this test if it is a link to the same file as filename. (the same inode index and is in the same filesystem as filename)

### Features
-For each file for which the test is true, it prints the relative pathname of the file with respect to the top level directory of the search <br>
-By default, this command will not follow symbolic links. When it examines a symbolic link, it tests the link itself, not its target <br>
-Only one test can be given to sfind <br>

### Defects/Shortcomings
-This one is partially working.
  - The -s option does not work. I believe the issues lies in something surroudning the pathname and not parsing that correctly
  - The -m option works
  - I think there is an issue with in general with the pathnames so Im no sure as of right now but maybe ill fix it

Thank you to Professor Weiss for the assigment!
