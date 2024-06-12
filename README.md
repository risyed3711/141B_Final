# SP23-141b-Database
## Sprint #1 -- Handling App-Level Commands

### Due April 8, 2024 at 11:30pm (PST)
Getting started with our application

## Getting Aquainted With the Code
In this assignment, you're expected to correctly process three simple commands ("about", "version", "quit", "help"). 

Take some time this week to review the code we've provided for you. All the files are intended to serve as the basis for your own database application. 

<b>NOTE</b>: While we've provided you some starting code and suggest you take advantage of it, you ARE NOT required to do so.
You're welcome to use your own code instead. You're also welcome to make any changes you like the files we've provided. 
Of course, any files you add to your repository must be original work (created by you or your teammate) -- you are not permitted to use code from another source.  

In order for your `AppController` to work with our testing system, it must provide the following methods:

```
virtual StatusResult  handleInput(std::istream &anInput, ViewListener aViewer);
OptString             getError(StatusResult &aResult) const;
bool                  running() const; //return false if issues 'quit' command
```


### We're using C++ 17 in this class

Make sure you have an IDE (editor, debugger) that generates code according to the C++ 17 standard. 
Visual Studio and XCode can both do this. You're going to be expected to work with new idioms that are only available in C++ 17.
CMakeLists.txt provided is set up to use C++ 17. You can use it directly with CLion or Visual Studio. To use it with XCode,
you can run 
```bash
cmake -G Xcode -B build
```
This will generate an XCode project that you can open and run in the build directory.

### Github project

Once you accept the assignment you should create a Github Project for your team. Here is the video created by Stephen showing how to create a project. 

[![How to create Github Project](https://img.youtube.com/vi/ytQ9RbqCf8U/0.jpg)](https://www.youtube.com/watch?v=ytQ9RbqCf8U)

Once the project is created, you should fill the Github_project.json

## Challenge #1 - Basic Command Processing

Your first challenge is to develop a way to 'process commands' that you recieve (in string form) from a user (via console or script). 
Your system will be called by the testing system via the `AppController::handleInput` method. Keep in mind that you'll 
recieve many types of commands, with many variations. Do not presume commands are always well formed, in a given case, or specific order. 
It may be wise to think about a flexible design -- rather than hard-coding a simple switch statement.

> Stop for a moment and this about what design patterns are at work here. 

You will be expected to identify the type of each command you recieve, process that command, and create a view object 
the represents the output of the command. If an error occurs (or you can't process the command), the view your repond 
with will describe the error. You'll pass the view along to a `ViewListener` so that your output can be presented.  

Begin by developing your own process for handling commands, which begins in the `AppController::handleInput(...)` method.

## Challenge #2 - The `About` Command

The first actual command you will handle is the "About" command. When issued, the `about` command will generate a response 
that includes the names of the members of the team that worked on your solution. Below is a sample interaction:

```
about;
Authors: Snoopy and Woodstock
```

Once again, your response to any command will be a `View` object of some type, that implements a `View.show()` method,
which when called and output the results of the command to an output stream.

## Challenge #3 -  The `Version` Command

Your next challenge is to implement the "version" command. When a user types the "version" command, your system should 
respond with the current version of your application, which is "0.1". Your output view should report:  "Version 0.1". 
Each week, the version number will be incremented by 0.01, until it reaches 1.0 in week 10.

As before, your response to any command will be a `View` object of some type, that implements a `View.show()` method, 
which when called and output the results of the command to an output stream. Sample output is shown below:

```
version;
Version: 0.1
```

## Challenge #4 - The `Quit` Command

Your next challenge is to implement the "quit" command. When issued, the result of this command is to produce a view that indicates the system is shutting down, and to change the state of your `AppController.running` value to false. In manual-testing mode, this will cause your app to terminate the control loop and stop processing commands. Sample output is shown below:

```
quit;
DB::141 is shutting down
```

Once the user has issused the `quit` command, your `AppController` should refuse to process any further commands.

## Challenge #5 - The `help` Command

Your next challenge is to implement the "help" command. When a user types the "help" command, your system should
respond with a list of commands that are available to the user, along with a brief description of each command. 
Your output view should report: 
```
help;
Available commands:
  about:    Display the authors of the program, usage: about;
  version:  Display the version of the program, usage: version;
  quit:     Quit the program, usage: quit;
  help:     Display this list of commands, usage: help;
```

## Challenge #6 - Manual Testing Mode 

This challenge should work automatically, presuming the others have been implemented. In this challenge, you will run 
your application in "interactive mode" by running your program without any arguments. In the `main.cpp` file, this 
scenario is handled by entering, "manual testing mode" -- where the program works interactively with you as user via the 
console. You can manually enter commands that get sent to your `AppController` to be run.  Manual testing model will 
continue to operate, until the user issues the `quit` command, after which the program should terminate.

It's essential that you be able to run your system interactively this way. Please test that now.

## Engineering Notebook

From a design standpoint, describe the design patterns in use in the starting code for this application. 
For each pattern you identify, describe which class the pattern is found it, along with a brief description
of how it's used. You should be able to identify a minimum of 10 patterns. 


Engineering Notebook will be done at a seperate assignment, please accept the assignemnt from here https://classroom.github.com/a/qgRQgSCE

## Submitting your work

Check your files using git (and check the auto-grader output) provided by Vlad-the-compiler.

### Grading

```
compile test: 5pts
warning test: 5pts
github project: 10pts
about test:   20pts
version test: 20pts 
quit test:    20pts
help test:    20pts
```

