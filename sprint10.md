# SP23-141b-Database -- Sprint #9
#### Due Friday June 7, 2024 - 11:30pm (PST)

## Overview 

### Working with Sockets and Databases 70%
In this assignment, we will continue to expand our Database system by integrating a simulated socket connection into our operations.  This will allow our testing system to connect the DB over the connection, rather than creating its won appControlller.

You will be working with a provided `DBConnector` class to establish a connection to the database to send requests. We've also provided a `DBServer` class, which represents the database server. You can find both of these classes in the `Remoting.hpp` file.

You will modify your current `ScriptRunner` class to use the `DBConnector` class for database operations instead of interfacing with the AppController directly.


### Passing all the past tests 20%

You will need to pass all the tests from the previous sprints.

### Final Prep with Logging 10%

We have provided you with a `Logger` class that you can use to log messages to any stream source (e.g., `std::cout`, `std::cerr`, or a file). You can use this class to log messages from your database operations.

You will need to add some logging messages to your code to help you debug and understand what is happening in your database operations as a preparatory step for the final.

There is no automatic testing for this part of the assignment, on RETROSPECTIVE.MD explain where you added log statements and their levels

### Refactoring 5% Extra Credit

You will have an opportunity to refactor your code to prepare for the final. Explain what you did in the RETROSPECTIVE.MD file.

## A Few Changes
### Config 

When you connect your code to the `appController` using the `DBConnector` class, you need to do so over a specific channel. Usually this refers to a network address, socket, or some other type of connector. We've implemented a pseudo-filesocket for this purpose. 

The code that tries to make a `DBConnection` will get the connection string from the `Config` class.  Add the following statement to the `Config.hpp` file:

```
  static const std::string getConnectionString() {
    return "localhost";
  }
```

### `TestAutomatic`   

Test `TestAutomatic` class has been rewritten. All of our existing tests work, but if you see any issues, let us know.

There is a new test available in `TestAutomatic` called "all", which will run all the existing tests in sequence.

## Key Classes in This Assignment
You will find the following classes significant in this assignment:

### The `DBConnector` class
`DBConnector` is a class we have provided that represents a connection to the database. It is responsible for managing the actual communication with the database server over a simulated network socket.
This simulated network socket is in a struct called `Channel`.The `DBConnector` class is responsible for sending and receiving messages over this channel.
You do not need to modify this class, but you should try to understand how it operates.

### The `DBServer` class
`DBServer` represents the database server in our system. It maintains a map of connections to different channels and is responsible for handling incoming messages from these channels.
You do not need to modify this class, but you should try to understand how it operates.

### Modifying the `ScriptRunner` Class
- Modify the `ScriptRunner` so that it will use the `DBConnector` class instead of the `AppController` class
- Find the line where commands are send to the `AppController` and replace it with a call to a method in `DBConnector` 

Once you have a valid connection, you will send commands over the `DBConnector` like this:

```
theResult=connector.send(theStream.str(), [&](View &aView) {
  aView.show(anOut);
  anOut << std::endl;
});
```

## Turning in your work by Friday 7th, 2024 - 11:30pm (PST)
Make sure your code compiles, and meets the requirements given above.
