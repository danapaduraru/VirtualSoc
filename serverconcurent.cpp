#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sqlite3.h>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>
#include <ctime>

using namespace std;
using Users = vector<string>;

#define PORT 2024
extern int errno;

void clearMsg(char msg[]){
    bzero(msg,200);
    fflush(stdout);
}

static int callback(void *unused, int count, char **data, char **columns) {
    Users* list = static_cast<Users*>(unused);
    for(int i=0; i < count; i++){
        list->push_back(data[i]);
    }
    return 0;
}

void SearchPost(int client, sqlite3* db, string id_user)
{
  Users list;
  string sqlcommand;
  int exc_cmd;
  string strposts;
  char keyword[200];
  read(client, keyword, 200);
  // get client's logname
  sqlcommand = "select logname from user where id = '" + id_user + "';";
  exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),callback,&list,NULL);
  string this_user_logname = list[0];
  list.clear();
  // get posts that have keyword in post text
  sqlcommand = "select distinct p.id, u.logname, p.created_date, p.text from post p join usergroups g on g.id_user = p.id_author join user u on g.id_user = u.id join friendslists f on g.id = f.id_group where f.id_user = '"
                + this_user_logname + "' and p.privacy = g.id_group and p.text like '%" + string(keyword) + "%'" +
                " UNION select distinct p.id, u.logname, p.created_date, p.text from post p join usergroups g on g.id_user = p.id_author join user u on g.id_user = u.id where p.privacy = 0 and p.text like '%" + string(keyword);
  sqlcommand += "%' UNION  select distinct p.id, u.logname, p.created_date, p.text from post p join usergroups g on g.id_user = p.id_author join user u on g.id_user = u.id where p.id_author = '" + id_user + "' and p.text like '%"
              + string(keyword) + "%';";
  exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),callback,&list,NULL);
  int size = list.size();
  if(size >= 1)
  {
    int i=0;
    while(i < list.size())
    {
      strposts  = strposts + "\n#" + list[i] + "\n" + list[i+2] + "\nby " + list[i+1] + "\n" + list[i+3] + "\n";
      i = i+4;
    }
  }
  char chrposts[40000];
  clearMsg(chrposts);
  strcpy(chrposts, strposts.c_str());
  if(strlen(chrposts) == 0 )
    strcpy(chrposts, "There are no posts that contain this keyword.\n");
  write(client, chrposts, 40000);
}

void AdminMenu(int client, sqlite3* db)
{
  Users list;
  string sqlcommand;
  int exc_cmd;
  string strposts;
  // get option from client
  char opt[200];
  clearMsg(opt);
  read(client, opt, 200);
  if(opt[0] == '1')
  {
    string strposts;
    list.clear();
    sqlcommand = "select p.id, u.logname, p.created_date, p.privacy, p.text from post p join user u on p.id_author = u.id order by p.id desc LIMIT 50;";
    exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),callback,&list,NULL);
    int i=0;
    while(i < list.size())
    {
      strposts  = strposts + "\n#" + list[i] + "\n" + list[i+2] + "\nby " + list[i+1] + "\n" + "privacy: " + list[i+3] + "\n" + list[i+4] + "\n";
      i = i+5;
    }
    char allposts[40000];
    clearMsg(allposts);
    strcpy(allposts, strposts.c_str());
    write(client, allposts, 40000);
    cout << "Admin #" << client << " accessed All Posts.\n";
  }
  else if(opt[0] == '2')
  {
    string strpost;
    list.clear();
    // get post ID
    char postID[200];
    read(client, postID, 200);
    sqlcommand = "select p.id, u.logname, p.created_date, p.privacy, p.text from post p join user u on p.id_author = u.id where p.id = " + string(postID) + " order by p.id LIMIT 50;";
    exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),callback,&list,NULL);
    int i=0;
    while(i < list.size())
    {
      strpost  = strpost + "\n#" + list[i] + "\n" + list[i+2] + "\nby " + list[i+1] + "\n" + "privacy: " + list[i+3] + "\n" + list[i+4] + "\n";
      i = i+5;
    }
    char post[800];
    clearMsg(post);
    if(strpost.length() == 0 ) strcpy(post, " There is no post with this ID.\n");
    else strcpy(post, strpost.c_str());
    write(client, post, 800);
    if(strpost.length() != 0 )
    {
      char saveopt[200];
      read(client, saveopt, 200);
      if(saveopt[0] == '1')
      {
        // delete post
        list.clear();
        sqlcommand = "delete from post where id = " + string(postID) + ";";
        exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),0,NULL,NULL);
        cout << "Client Admin #" << client << " deleted post with ID " << postID << ".\n";
      }
    }
  }
  else if(opt[0] == '3')
  {
    char message[200];
    char user_logname[200];
    clearMsg(user_logname);
    string userID;
    list.clear();
    read(client, user_logname, 200);
    // get user's DB ID
    sqlcommand = "select id from user where logname = '" + string(user_logname) + "';";
    exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),callback,&list,NULL);
    int list_size = list.capacity();
    if(list_size >=1)
      userID = list[0];
    else
      strcpy(message, "There is no user with this ID.\n");
    char saveopt[200];
    read(client, saveopt, 200);
    if(list_size >=1)
    {
      if(saveopt[0] == '1')
      {
        // delete all of his posts
        sqlcommand = "delete from post where id_author = " + userID + ";";
        exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),0,NULL,NULL);
        // delete user from Friend Lists
        sqlcommand = "delete from FriendLists where id_user = '" + string(user_logname) + "';";
        exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),0,NULL,NULL);
        // delete user from UserGroups
        sqlcommand = "delete from UserGroups where id_user = " + userID + ";";
        exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),0,NULL,NULL);
        // delete user from User
        sqlcommand = "delete from user where id = " + userID + ";";
        exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),0,NULL,NULL);
        strcpy(message, "User has been deleted.\n");
        cout << "Client Admin # " << client << " has deleted user with ID " << userID << ".\n";
      }
      else
        strcpy(message, "No changes were made.\n");
    }
    write(client, message, 200);
   }
  else if(opt[0] == '4')
  {
     char message[200];
     char user_logname[200];
     clearMsg(user_logname);
     string userID;
     list.clear();
     read(client, user_logname, 200);
     // get user's DB ID
     sqlcommand = "select id from user where logname = '" + string(user_logname) + "';";
     exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),callback,&list,NULL);
     int list_size = list.capacity();
     if(list_size >=1)
       userID = list[0];
     else
       strcpy(message, "There is no user with this ID.\n");
     char saveopt[200];
     read(client, saveopt, 200);
     if(list_size >=1)
     {
       if(saveopt[0] == '1')
       {
         sqlcommand = "UPDATE USER SET isAdmin = 1 WHERE ID = " + userID + ";";
         exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),0,NULL,NULL);
         strcpy(message, "User is now an admin.\n");
         cout << "Client Admin # " << client << " made user with ID " << userID << " an admin.\n";
       }
       else
         strcpy(message, "No changes were made.\n");
     }
     write(client, message, 200);
    }
}

bool CheckIfAdmin(int client, sqlite3* db, string id_user)
{
  Users list;
  string sqlcommand;
  int exc_cmd;
  string strposts;
  sqlcommand = "select isAdmin from user where id ='" + id_user + "';";
  exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),callback,&list,NULL);
  if(list[0] == "1")
    return true;
  else return false;
}

void MyPosts(int client, sqlite3* db, string id_user)
{
  Users list;
  string sqlcommand;
  int exc_cmd;
  string strposts;
  char message[40000];
  clearMsg(message);
  sqlcommand = "select p.id, u.logname, p.created_date, p.text from post p join user u on p.id_author = u.id where u.id='" + id_user + "' order by p.id LIMIT 50;";
  exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),callback,&list,NULL);
  int size = list.size();
  if(size >=1)
  {
    int i=0;
    while(i < list.size())
    {
      strposts  = strposts + "\n#" + list[i] + "\n" + list[i+2] + "\nby " + list[i+1] + "\n" + list[i+3] + "\n";
      i = i+4;
    }
    strcpy(message, strposts.c_str());
  }
  else strcpy(message, "You haven't made any posts yet.\n");
  write(client, message, 40000);
  cout << "Client #" << client << " accessed My Posts.\n";
}

void MyGroups(int client, sqlite3* db, string id_user)
{
  string group_title;
  Users list;
  string sqlcommand;
  int exc_cmd;
  string strposts;
  string group_number; // for sqlcommand
  char msg[200];
  clearMsg(msg);
  read(client, msg, 200);   // receive option
  if(msg[0] == '1'){
    group_title = "Friends";
    group_number = "1";
  }
  else if(msg[0] == '2'){
    group_title = "Close Friends";
    group_number = "2";
  }
  else if(msg[0] == '3'){
    group_title = "Mates";
    group_number = "3";
  }
  if(msg[0] != '4') // CHOOSING THE GROUP
  {
    cout << "Client #" << client << " accessed " << group_title << endl;
    clearMsg(msg);
    list.clear();
    // print Posts that were made by users who have added client as Friend/Close Friend/Mate
    sqlcommand = "select logname from user where id = '" + id_user + "';";
    exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),callback,&list,NULL);
    string this_user_logname = list[0];
    list.clear();
    sqlcommand = "select p.id, u.logname, p.created_date, p.text from post p join usergroups g on g.id_user = p.id_author join user u on g.id_user = u.id join friendslists f on g.id = f.id_group where f.id_user = '"
                  + this_user_logname + "' and g.id_group = " + group_number + " and p.privacy = " + group_number + "  order by p.id LIMIT 50; ;";
    exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),callback,&list,NULL);
    int i=0;
    while(i < list.size())
    {
      strposts  = strposts + "\n#" + list[i] + "\n" + list[i+2] + "\nby " + list[i+1] + "\n" + list[i+3] + "\n";
      i = i+4;
    }
    char chrposts[40000];
    clearMsg(chrposts);
    strcpy(chrposts, strposts.c_str());
    if(strlen(chrposts) == 0 )
      strcpy(chrposts, "There aren't any posts yet.\n");
    write(client, chrposts, 40000);
    read(client, msg, 200);
    if(msg[0] == '1')
    {
      // see list of friends in this group
      list.clear();
      sqlcommand = "select ID from UserGroups where id_group = " + group_number + " and id_user = " + id_user + ";";
      exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),callback,&list,NULL);
      string id_friendsgroup = list[0];
      list.clear();
      sqlcommand = "select id_user from FriendsLists where id_group = " + id_friendsgroup + ";";
      exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),callback,&list,NULL);
      string strfriends_list;
      for(auto i : list)
      {
        strfriends_list += i;
        strfriends_list += "\n";
      }
      char friends_list[600];
      clearMsg(friends_list);
      strcpy(friends_list, strfriends_list.c_str());
      if(strlen(friends_list) == 0 )
        strcpy(friends_list, "\nYou haven't added any friends yet.\n");
      write(client, friends_list, strlen(friends_list));
      // get option friends menu
      char msg_friends[200];
      clearMsg(msg_friends);
      read(client, msg_friends, 200);
      if(msg_friends[0] != '3')
      {
        char id_friend[300];
        char added_message[300];
        clearMsg(id_friend);
        clearMsg(added_message);
        read(client, id_friend, 300);
        if(msg_friends[0] == '1') // ADD FRIEND
        {
          // verify if logname exists
          list.clear();
          sqlcommand = "select count(*) from user where logname = '" + string(id_friend) + "';";
          exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),callback,&list,NULL);
          if(list[0] != "0")
          {
            // logname exists in database => we can add them as friends. See explanation for FriendsLists in the comment at LINE 269
            list.clear();
            sqlcommand = "select ID from UserGroups where id_group = " + group_number + " and id_user = " + id_user + ";";
            exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),callback,&list,NULL);
            string id_friendsgroup = list[0];
            // verify if friend is already added
            list.clear();
            sqlcommand = "select count(*) from FriendsLists where id_group = " + id_friendsgroup + " and id_user = '" + string(id_friend) + "';";
            exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),callback,&list,NULL);
            if(list[0] == "0")
            {
              list.clear();
              sqlcommand = "INSERT INTO FriendsLists(id_group, id_user) VALUES ('" + id_friendsgroup + "', '" + string(id_friend) + "');";
              exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),0,NULL,NULL);
              cout << "Client #" << client << " has added a new friend in " << group_title << "." << endl;
              strcpy(added_message, "Your new friend has been added!\n");
              write(client, added_message, 300);
            }
            else
            {
              strcpy(added_message, "This user is already on your list in this group.\n");
              write(client, added_message, 300);
            }
          }
          else
          {
            strcpy(added_message, "Sorry, we couldn't find anyone with his ID in our database.\n");
            write(client, added_message, 300);
          }
        }
        else if(msg_friends[0] == '2') // SEE FRIEND'S PROFILE
        {
          // verify if friend is found in list
          list.clear();
          sqlcommand = "select count(*) from FriendsLists where id_group = " + id_friendsgroup + " and id_user = '" + id_friend + "';";
          exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),callback,&list,NULL);
          char details[600];
          clearMsg(details);
          if(list[0] != "0")
          {
            // find user details by logname
            list.clear();
            sqlcommand = "select name, surname, logname, occupation, birthday, description from user where logname = '" + string(id_friend) + "';";
            exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),callback,&list,NULL);
            // Send details to client in one big char[]
            string dstring = list[0] + " " + list[1] + "\n" + list[2] + "\n\n" + list[3] + "\nBirthday: " + list[4] + "\nDESCRIPTION\n" + list[5] + "\n";
            strcpy(details, dstring.c_str());
            cout << "Client #" << client << " accessed " << id_friend << "'s profile.\n";
            write (client, details, 600);
          }
          else
          {
            strcpy(details, "There is no one with this ID in your ");
            strcat(details, group_title.c_str());
            strcat(details, " friends list.\n");
            write (client, details, 600);
          }
        }
      }
    }
  }
}

void Newsfeed(int client, sqlite3* db)
{
  Users list;
  string sqlcommand;
  int exc_cmd;
  string strposts;
  // get the 50 most recent public posts
  sqlcommand = "select p.id, u.logname, p.created_date, p.text from post p join user u on p.id_author = u.id where p.privacy = 0 order by p.id LIMIT 50;";
  exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),callback,&list,NULL);
  int i=0;
  while(i < list.size())
  {
    strposts  = strposts + "\n#" + list[i] + "\n" + list[i+2] + "\nby " + list[i+1] + "\n" + list[i+3] + "\n";
    i = i+4;
  }
  char message[40000];
  clearMsg(message);
  strcpy(message, strposts.c_str());
  write(client, message, 40000);
  cout << "Client #" << client << " accessed Newsfeed.\n";
}

void NewPost(int client, string id_user, sqlite3* db)
{
  char post[800];
  clearMsg(post);
  read(client, post, 800);
  char privacy[200];
  clearMsg(privacy);
  read(client, privacy, 2);
  char save_opt[200];
  clearMsg(save_opt);
  read(client, save_opt, 2);
  if(save_opt[0] == '1')
  {
    // save post to db
    auto start  = std::chrono::system_clock::now(); // get created date for post
    std::time_t time = std::chrono::system_clock::to_time_t(start);
    string created_date = string(ctime(&time));
    created_date.pop_back(); // removes last character from string. Had an '\n'
    Users list;
    string sqlcommand;
    int exc_cmd;
    sqlcommand = "INSERT INTO Post(id_author, created_date, text, privacy) VALUES ('" + id_user + "','" + created_date + "',\"" + string(post) + "\",'"
                  + string(privacy) + "');";
    exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),0,NULL,NULL);
    cout << "Client #" << client << " has created a new post.\n";
  }
}

void ChangeProfile(int client, string id_user, sqlite3* db, char choice[])
{
  Users list;
  string sqlcommand, field;
  int exc_cmd;
  switch(choice[0])
  {
    case '1': field = "Name"; break;
    case '2': field = "Surname"; break;
    case '3': field = "Occupation"; break;
    case '4': field = "Birthday"; break;
    case '5': field = "Description"; break;
  }
  sqlcommand = "SELECT " + field + " FROM user WHERE ID = " + id_user + ";";
  exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),callback,&list,NULL);
  // receive modified field
  char new_field[300];
  clearMsg(new_field);
  read(client, new_field, 300);
  // receive save choice
  char new_field_save[300];
  clearMsg(new_field_save);
  read(client, new_field_save, 300);
  if(new_field_save[0] == '1')
  {
    sqlcommand = "UPDATE USER SET " + field + " = '" + new_field + "' WHERE ID = " + id_user + ";";
    exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),0,NULL,NULL);
    cout << "Client #" << client << " successfully modified their " << field << endl;
  }
}

void Login(int client, sqlite3 *db, int &logged, string &id_user)
{
  Users list;
  int exc_cmd;
  string sqlcommand, login_message;
  char logname[63]="", password[30]="", message[200]="";
  // receive logname and password from client
  read (client, logname, 63);
  read (client, password, 30);
  clearMsg(message);
  // verify if logname exists in database
  sqlcommand = "select count(*) from user where logname = '" + string(logname) + "';";
  exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),callback,&list,NULL);
  if(list[0] != "0")
  {
    // logname exists in database
    sqlcommand = "select password from user where logname = '" + string(logname) + "';";
    exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),callback,&list,NULL);
    if(list[1] == string(password))
    {
      // password is correct
      strcpy(message, "Login successful!\n\n");
      // get user's ID from db
      sqlcommand = "select id from user where logname = '" + string(logname) + "';";
      exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),callback,&list,NULL);
      id_user = list[2];
      logged = 1;
    }
    else strcpy(message, "Wrong password!\n\n");
  }
  else strcpy(message, "ID was not found in the database.\n");
  write (client, message, 40);   // send login status message
}

void AddAccount(sqlite3 *db, int client)
{
  Users list;
  int exc_cmd;
  string sqlcommand;
  char name[30], surname[30], logname[63], password[30];
  // Get user details
  read (client, name, 30); name[strlen(name)]='\0';
  read (client, surname, 30); surname[strlen(surname)]='\0';
  read (client, password, 30); password[strlen(password)]='\0';
  read (client, logname, 63); logname[strlen(logname)]='\0';
  if( strlen(name)!=0 && strlen(surname)!=0 && strlen(password)!=0 && strlen(logname)!=0)
  {
    // Verify if another user with this ID already exists.
    sqlcommand = "select count(*) from user where name = '" + string(name) + "' and surname = '" + string(surname) + "';";
    exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),callback,&list,NULL);
    if(list[0] != "0")
    {
      // create new username in this format: name.surname01, name.surname02...
      string lognamestring = string(logname) + list[0];
      strcpy(logname,lognamestring.c_str());
    }
    // INSERT USER INFO INTO DATABASE
    sqlcommand = "INSERT INTO User(logname, name, surname, password, occupation, birthday, description, isAdmin) VALUES ('"
                  + string(logname)+ "','" + string(name)+ "','" + string(surname)+"','"+ string(password)+"','-','00/00/0000','No description yet.','0');";

    exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),0,NULL,NULL);
    cout << "[server] Client #" << client << " with logname " << logname << " has been added to the database.\n";
    write (client, logname, 63);   // send ID to client
    list.clear();
    /* in table UserGroups, we will have 3 records for each user ID with each group ID. For example, for two users with IDs 5 and 8 we'll have these records:
    ID  GroupID UserID
    1     1       5
    2     2       5
    3     3       5
    4     1       8
    5     2       8
    6     3       8
    in table FriendsLists we'll have, for example, this record:
    ID  GroupID  UserID
    1     6        5
    This means that user with ID 5 is mates with the user that has the group ID 6, which is actually groupID = 3 for the user with ID 8.
    In other words, User 8 has added to his mates group the user with ID 5. THis does not mean that user 5 has done the same thing for user 8.
    */
    sqlcommand = "select id from user where logname = '" + string(logname) + "';";
    exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),callback,&list,NULL);
    string id_user = list[0];
    list.clear();
    sqlcommand = "INSERT INTO UserGroups(id_group, id_user) VALUES ('1', '" + id_user + "');";
    exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),0,NULL,NULL);
    sqlcommand = "INSERT INTO UserGroups(id_group, id_user) VALUES ('2', '" + id_user + "');";
    exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),0,NULL,NULL);
    sqlcommand = "INSERT INTO UserGroups(id_group, id_user) VALUES ('3', '" + id_user + "');";
    exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),0,NULL,NULL);
  }
}

void sighandler(int sig)
{
	if(sig==SIGCHLD)
		while(waitpid(-1,0,WNOHANG) > 0) { }
}

int main ()
{
  struct sockaddr_in server;	 // structura folosita de server
  struct sockaddr_in from;
  char m[200];								 //mesajul primit de la client
  int sd;											 //socket descriptor
	int child;
	sqlite3 *db;

	// Open Database
  if(sqlite3_open("virtualsoc.db", &db))
    cout << "Could not open database\n";

  // Create socket
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1){
		perror ("[server]ERR socket().\n");
    return errno;
  }

  // Pregatirea structurilor de date
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  // Umplem structura folosita de server
  server.sin_family = AF_INET;									  //stabilirea familiei de socket-uri
  server.sin_addr.s_addr = htonl (INADDR_ANY);		// acceptam orice adresa
  server.sin_port = htons (PORT);									// utilizam un port utilizator

  // Attach socket
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1){
		perror ("[server]ERR bind().\n");
    return errno;
  }

	if(signal(SIGCHLD,sighandler) == SIG_ERR)  printf("It's ok");
  // Server listens for clients who are willing to connect
  if (listen (sd, 5) == -1){
		perror ("[server]ERR listen().\n");
    return errno;
  }
  // Servim in mod CONCURENT clientii
  while (1)
  {
		int client;
		socklen_t length = sizeof (from);
		printf ("[server]Waiting at port %d...\n",PORT);
		fflush (stdout);
    // Acceptam un client (stare blocanta pina la realizarea conexiunii)
    client = accept (sd, (struct sockaddr *) &from, &length);
		if ((child = fork()) == -1)
			perror("Err...fork");
    else if (child==0)   //child
		{
      if (client < 0){
				perror ("[server]ERR accept().\n");
	  		continue;
			}
      cout << "Client #" << client << " CONNECTED\n";
      // S-a realizat conexiunea, se astepta mesajul
      clearMsg(m);
      // Read option from client
			if (read (client, m, 100) <= 0){
				perror ("[server]ERR read() welcome message from client.\n");
	  		close (client);
	  		continue;
			}
			if(m[0] == '1')  // LOGIN
			{
        cout << "Client #" << client << " attempts login.\n";
				string id_user;
				int logged = 0;
				Login(client, db, logged, id_user);
				if(logged == 1)
				{
					// client has logged in => has access to MAIN MENU
					// first, send to client his ID (not logname!) - we're gonna use it for functions in client
					char msg[300];
					clearMsg(msg);
					strcpy(msg, id_user.c_str());
					write(client,msg,300);
					cout << "Client #" << client << " with ID " << id_user <<" has successfully logged in.\n";
          // second, check if user is admin or not
          bool isAdmin = CheckIfAdmin(client, db, id_user);
          char msg_admin[3];
          clearMsg(msg_admin);
          if(isAdmin) strcpy(msg_admin, "1");
          else strcpy(msg_admin, "0");
          write(client,msg_admin,3);
					while(true)
					{
						// read option from client
						clearMsg(m);
						read (client, m, 100);
						if(m[0] == '1'){
		          NewPost(client, id_user, db);
		        }
		        else if(m[0] == '2'){
		          Newsfeed(client, db);
		        }
		        else if(m[0] == '3')
		        {
							// find user details by logname
							char details[600];
							int exc_cmd;
							string sqlcommand;
              Users list;
							list.clear();
							sqlcommand = "select name, surname, logname, occupation, birthday, description from user where id = '" + id_user + "';";
							exc_cmd = sqlite3_exec(db,sqlcommand.c_str(),callback,&list,NULL);
							// Send details to client in one big char[]
							clearMsg(details);
							string dstring = list[0] + " " + list[1] + "\n" + list[2] + "\n\n" + list[3] + "\nBirthday: " + list[4] + "\nDESCRIPTION\n";
							dstring += list[5];
              strcpy(details, dstring.c_str());
							cout << "Client #" << client << " accessed My Profile.\n";
							write (client, details, 600); // dunno why it doesn't work once
							write (client, details, 600); // dunno why it doesn't work once
							// receive profile choice - change profile or back to MENU
							char choice[200];
							clearMsg(choice);
							read(client, choice, 200);
							if(choice[0] == '1')
							{
                char change_profile_choice[200];
  							clearMsg(change_profile_choice);
  							read(client, change_profile_choice, 200);
                ChangeProfile(client, id_user, db, change_profile_choice);
							}
		        }
            else if(m[0] == '4'){
		          MyPosts(client, db, id_user);
		        }
		        else if(m[0] == '5'){
		          MyGroups(client, db, id_user);
		        }
		        else if(m[0] == '6'){
		          SearchPost(client, db, id_user);
		        }
            else if(m[0] == '7'){
              cout << "Client #" << client << " with ID " << id_user <<" has exited the application.\n";
		          break;
            }
            else if(m[0] == '8'){
              cout << "Client #" << client << " accessed Admin Menu\n";
              AdminMenu(client, db);
            }
		        clearMsg(m);
					}
					close(client);
				}
			}
			else if(m[0] == '2'){
        cout << "Client #" << client << "accessed Newsfeed.\n";
        Newsfeed(client, db);
			}
			else if(m[0] == '3'){
				cout << "Client #" << client << " wants to create an account.\n";
				AddAccount(db, client);
			}
			else if(m[0] == '4'){
        cout << "Client #" << client << "accessed Details.\n";
			}
			else if(m[0] == '5'){
				cout << "[server] Client #" << client << " has exited the application.\n";
				close(client);
			}
      close (client);
			exit(0);
		} // if child
  } // while
}
