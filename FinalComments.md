I was able to successfully complete the backup/restore portion of the final. I am successfully able to
both backup my code to a .sql file, and then restore it and get the exact same working copy of the database
that I had before. 

The next challenge I chose to tackle was the count function. I did this first by implementing a parseCount
method in my DBHandler clause parser. This integrated very nicely into my parsing scheme, and seems to be
working well. The part where I got stuck was right here, due to time constraints of not being able to implement
more than this after plannning. If I had more time, I would then move on to simply changing the way the view for the 
select command is constructed given that the count function was being called. I would do this by checking
with an if statement before any of the rest of the handleSelectCommand view creation happened, in which 
case I would create a singular table with the column name that was chosen before doing my processing. To 
fill in the table with the correct number in the view, I would make sure to traverse my entire rowcollection 
of objects, which I would have already successfully obtained from muy backend through the relevant "from" 
schema. For each row in this rowcollection which had the wanted value that was not equal to null, I would increment a counter, 
finally adding this final counter to the table and returning my results. 

I would have chosen to do the second challenge last. In order to do this, I would first have done validation in 
the create table command which would make sure that BabyGIS was activated, before allowing a user to input 
BabyGIS data type into the table that they were creating as an attribute. I would make the activation scheme rather simple, 
by having a bool "babyGISActivated" per each database on a database level, so that I would know for what 
databases it was and wasnt active for. After this, I would be able to know whether or not to allow for BabyGIS or 
not. Finally, when it comes to adding the BabyGIS data after explicitly specifying whether it is allowed or not, 
I would have added it to the Value Variant. For storing/retreiving it, I would've treated it as 6 doubles stored
contiguously in memory, which is how I would've been able to read and write it to memory in the backend. This would
not be as hard to integrate, I would just do this exactly where I am encoding things like strings and ints, ie the other
types supported by my Value Variant from throughout the quarter. Finally, for selecting BabyGIS data, 
I would've added a parser for it in the DB level, and then added it to the view. 

Thank you for the quarter!