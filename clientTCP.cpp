#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>
using namespace std;
extern int errno;
int port;

void clearMsg(char msg[]){
    bzero(msg,200);
    fflush(stdout);
}

void verifyOption(int nrOpt, char msg[])
{
  bool ok = 0;
  while(true)
  {
      clearMsg(msg);
      read (0, msg, 200);
      msg[strlen(msg)-1] = '\0';
      for(int i = 1; i <= nrOpt; i++)
          if(string(msg) == to_string(i))
              ok = 1;
      if(ok == 0)
          cout << "\nPlease choose one of the options above!\n";
      else break;
    }
    msg[strlen(msg)]='\n';
}

void AdminMenu(int sd)
{
  printf("\033c");
  cout << "\n Application Management\n\n (1) See most recent 50 posts\n (2) Delete post\n (3) Delete user\n (4) Make Admin\n (5) Back to menu\n\n Enter your option: ";
  char opt[200];
  verifyOption(5, opt);
  write(sd, opt, 200);
  if(opt[0] == '1')
  {
    char allposts[40000];
    clearMsg(allposts);
    read(sd, allposts, 40000);
    cout << allposts << endl;
  }
  else if(opt[0] == '2')
  {
    cout << " Enter post's ID: ";
    char postID[200];
    clearMsg(postID);
    cin.getline(postID,200);
    write(sd, postID, 200);
    char post[800];
    clearMsg(post);
    read(sd, post, 800);
    cout << post << endl;
    if(strcmp(post, " There is no post with this ID.\n") !=  0)
    {
      cout << " Are you sure you want to delete this post?\n (1) Yes\n (2) No\n";
      char saveopt[200];
      verifyOption(2, saveopt);
      write(sd, saveopt, 200);
      if(saveopt[0] == '1')
        cout << " Post has been deleted.\n";
      else if(saveopt[0] == '2') cout << " Post was not deleted.\n";
    }
  }
  else if(opt[0] == '3')
  {
    cout << " Enter user's ID: ";
    char userID[200];
    clearMsg(userID);
    cin.getline(userID,200);
    write(sd, userID, 200);
    cout << " Are you sure you want to delete this user and all of his posts?\n (1) Yes\n (2) No\n";
    char saveopt[200];
    verifyOption(2, saveopt);
    write(sd, saveopt, 200);
    char message[200];
    clearMsg(message);
    read(sd, message, 200);
    cout << message << endl;
  }
  else if(opt[0] == '4')
  {
    cout << " Enter user's ID: ";
    char userID[200];
    clearMsg(userID);
    cin.getline(userID,200);
    write(sd, userID, 200);
    cout << " Are you sure you want to make this user an admin?\n (1) Yes\n (2) No\n";
    char saveopt[200];
    verifyOption(2, saveopt);
    write(sd, saveopt, 200);
    char message[200];
    clearMsg(message);
    read(sd, message, 200);
    cout << message << endl;
  }
}

void MyGroups(int sd)
{
  char msg[200];
  string group_title;
  clearMsg(msg);
  printf("\033c");
  cout << "\nMy Groups\n\n(1) Friends\n(2) Close Friends\n(3) Mates\n(4) Back to menu\n\n";
  verifyOption(4,msg);
  write(sd, msg, 200);
  if(msg[0] == '1') group_title = "Friends";
  else if(msg[0] == '2') group_title = "Close Friends";
  else if(msg[0] == '3') group_title = "Mates";
  if(msg[0] != '4')
  {
    printf("\033c");
    clearMsg(msg);
    cout << "Posts by other friends:\n";
    char posts[40000];
    clearMsg(posts);
    read(sd, posts, 40000);
    cout << posts << endl;
    cout << endl << group_title << "\n\n(1) See Friends\n(2) Back to menu\n";
    cout << "\nEnter your option: ";
    verifyOption(2,msg);
    write(sd, msg, 200);
    if(msg[0] == '1')
    {
      printf("\033c");
      cout << "\n" << group_title << " list\n";
      char friends_list[600];
      clearMsg(friends_list);
      read(sd, friends_list, 600);
      cout << endl << "(1) Add Friend\n(2) See Friend's Profile\n(3) Back to menu\n\n";
      cout << friends_list;
      cout << "\nEnter your option: ";
      char msg_friends[200];
      clearMsg(msg_friends);
      verifyOption(3,msg_friends);
      write(sd, msg_friends, 200);
      if(msg_friends[0] != '3')
      {
        char id_friend[300];
        clearMsg(id_friend);
        cout << "Enter friend's ID: ";
        while(true)
        {
          cin.getline(id_friend,300);
          if(strlen(id_friend) == 0)
            cout << "Enter friend's ID: ";
          else break;
        }
        write(sd, id_friend, 300);
        if(msg_friends[0] == '1')
        {
          char added_message[300];
          clearMsg(added_message);
          read(sd, added_message, 300);
          cout << added_message << endl;
        }
        else if(msg_friends[0] == '2')
        {
          printf("\033c"); // CLEAR SCREEN
          char user_details[600];
          clearMsg(user_details);
          // read details from server
          read (sd, user_details, 600);
          cout << user_details;
        }
      }
    }
  }
}

void NewPost(int sd)
{
  printf("\033c");
  char post[800];
  clearMsg(post);
  cout << "Write or paste your new post here. Press enter when you're finished!\nMax length: 800 characters.\n\n";
  cin.getline(post,800);
  while(strlen(post) < 1 || strlen(post) > 800)
  {
    cout << "Your post is either too short or too long: " << strlen(post) << " characters.\n";
    cin.getline(post,800);
  }
  write(sd, post, strlen(post));
  cout << "\nGreat! Set the privacy for your post:\n\n(1) Public\n(2) Friends\n(3) Close Friends\n(4) Mates\n\n";
  char privacy[200];
  clearMsg(privacy);
  verifyOption(4,privacy);
  switch(privacy[0])
  {
    case '1' : privacy[0]='0'; break;
    case '2' : privacy[0]='1'; break;
    case '3' : privacy[0]='2'; break;
    case '4' : privacy[0]='3'; break;
  }
  write(sd, privacy, 2);
  cout << "\n(1) Save\n(2) Cancel\n\n";
  char save_opt[200];
  clearMsg(save_opt);
  verifyOption(2,save_opt);
  write(sd, save_opt, 2);
  if(save_opt[0] == '1')
    cout << "Your options have been saved!\n";
  else
    cout << "Your post was not saved.\n";
}

void Login(int sd, int &logged)
{
  char logname[200], password[200], message[200];
  cout << "ID: ";
  clearMsg(logname);
  read(0,logname,63);
  cout << "Password: ";
  clearMsg(password);
  read(0,password,30);
  password[strlen(password)-1]='\0';
  logname[strlen(logname)-1]='\0';
  write(sd,logname,63);
  write(sd,password,63);
  // Receive login message
  read (sd, message, 40);
  if(strcmp(message,"Login successful!\n\n")==0)
    logged = 1;
  cout << message << endl;
}

char ModifyName(char name[])
{
  if(name[0] >= 97)
    name[0] = name[0] - 32;           // save name with capital letter
  for(int i=1; i < strlen(name); i++)
    if(name[i] < 97 && name[i]!=' ')
      name[i] = name[i] + 32;         // make all the other letters lower caps
}

void CreateAccount(int sd)
{
  char name[30], surname[30], logname[63], password[30];
  cout << "Please enter the following information:\nFirst Name: ";
  clearMsg(name);
  cin.getline(name,30);
  ModifyName(name);

  cout << "Last Name: ";
  clearMsg(surname);
  cin.getline(surname,30);
  ModifyName(surname);

  cout << "Password (at least 3 characters long): ";
  clearMsg(password);
  cin.getline(password,30);
  while(strlen(password) < 3)
  {
    cout << "Password is too short!\n";
    cout << "Password (at least 3 characters long): ";
    cin.getline(password,30);
  }
  // create copies used to generate ID
  char namecopy[30]="", surnamecopy[30]="";
  strcpy(namecopy, name);
  strcpy(surnamecopy, surname);
  namecopy[0] = namecopy[0] + 32;
  surnamecopy[0] = surnamecopy[0] + 32;
  strcat(logname, namecopy);
  strcat(logname, ".");
  strcat(logname, surnamecopy);
  //send info to server
  write(sd,name,30);
  write(sd,surname,30);
  write(sd,password,30);
  write(sd,logname,63);
  // receive ID from server
  char lognamenew[200];
  clearMsg(lognamenew);
  read(sd, lognamenew, 200);
  cout << "\nGreat!Use this ID to log in: \n" << lognamenew << endl;
}

int main (int argc, char *argv[])
{
  int sd;
  struct sockaddr_in server;

  if (argc != 3)
  {
    printf ("Syntax: %s <server_address> <port>\n", argv[0]);
    return -1;
  }
  port = atoi (argv[2]);   // stabilim portul

  // CREATE SOCKET
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror ("ERR socket().\n");
    return errno;
  }

  // Umplem structura folosita pentru realizarea conexiunii cu serverul
  server.sin_family = AF_INET;                  // socket's family
  server.sin_addr.s_addr = inet_addr(argv[1]);  // adresa IP a serverului
  server.sin_port = htons (port);               // connection port

  // CONNECT TO SERVER
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1){
    perror ("[client]ERR connect().\n");
    return errno;
  }
  printf ("Welcome to VirtualSoc! Choose one of the following:\n (1) Login\n (2) See all public posts\n (3) Create new account\n (4) Find out more about the application\n (5) Exit\n ");
  char opt[200];
  verifyOption(5,opt);
  write (sd, opt, 10);
  if(opt[0] == '1') // LOGIN
  {
    int logged = 0;
    Login(sd,logged);
    if(logged == 1)
    {
      // client has logged in => has access to MAIN MENU
      // first, get client's id from db
      char msg[300];
      clearMsg(msg);
      read (sd, msg, 300);
      string id_user = string(msg); // this is NOT the logname, it is the ID from the db.
      // second, check if user's admin
      char msg_admin[3];
      clearMsg(msg_admin);
      read (sd, msg_admin, 3);
      string isAdmin = string(msg_admin);
      cout << "\n VirtualSoc \n Welcome back!" << endl;
      while(true)
      {
        // Send chosen option
        cout << "\n\n (1) Create new post \n (2) Newsfeed\n (3) My Profile\n (4) My Posts\n (5) My Groups\n (6) Search Post\n (7) Exit";
        if(isAdmin[0] == '1') cout << "\n (8) App Management\n";
        cout << "\n\nEnter your option: \n";
        strcpy(opt,"");
        if(isAdmin[0] == '1') verifyOption(8,opt);
        else verifyOption(7,opt);
        write (sd, opt, 3);
        if(opt[0] == '1'){
          NewPost(sd);
        }
        else if(opt[0] == '2'){
          printf("\033c");
          cout << "\nNEWSFEED\n\n";
          char message[40000];
          clearMsg(message);
          read(sd, message, 40000);
          cout << message << endl;
        }
        else if(opt[0] == '3') // MYPROFILE
        {
          printf("\033c"); // CLEAR SCREEN
          char user_details[600];
          clearMsg(user_details);
          // read details from server
          read (sd, user_details, 600); // dunno why it doesn't work once
          read (sd, user_details, 600); // dunno why it doesn't work once
          cout << user_details << "\n\n(1) Change Profile\n(2) Back to menu\n\n" << endl;
          char choice[200];
          verifyOption(2,choice);
          write(sd, choice, 2);
          if(choice[0]=='1')
          {
            cout << "What would you like to change?\n\n(1) Name\n(2) Surname\n(3) Occupation\n(4) Birthday\n(5) Description\n\n";
            char change_profile_choice[200], new_field[300], new_field_save[200];
            verifyOption(5,change_profile_choice);
            write(sd, change_profile_choice,2);
            string field;
            switch(change_profile_choice[0])
            {
              case '1': field = "name"; break;
              case '2': field = "surname"; break;
              case '3': field = "occupation"; break;
              case '4': field = "birthday"; break;
              case '5': field = "description"; break;
            }
            cout << "Please type your new " << field << ":\n\n";
            while(true)
            {
              cin.getline(new_field,300);
              if(strlen(new_field) < 3)
                cout << "Please enter at least 3 characters.\n";
              else break;
            }
            write(sd, new_field, strlen(new_field));
            fflush(stdout);
            cout << "\n(1) Save\n(2) Cancel\n\n";
            verifyOption(2, new_field_save);
            write(sd, new_field_save, 2);
            if(new_field_save[0] == '1')
              cout << "Your changes have been saved!\n";
            else
              cout << "No changes made.\n";
          }
        }
        else if(opt[0] == '4'){
          printf("\033c");
          cout << "\nYour Posts\n\n";
          char message[40000];
          clearMsg(message);
          read(sd, message, 40000);
          cout << message << endl;
        }
        else if(opt[0] == '5'){
          MyGroups(sd);
        }
        else if(opt[0] == '6')
        {
          printf("\033c");
          char keyword[200];
          cout << "Search Post\n";
          cout << "Enter keyword here: ";
          cin.getline(keyword, 200);
          while(strlen(keyword) < 3)
          {
            cout << "Keyword should be at least 3 characters long.\n";
            cin.getline(keyword, 200);
          }
          write(sd, keyword, 200);
          char posts[40000];
          clearMsg(posts);
          read(sd, posts, 40000);
          cout << posts << endl;
        }
        else if(opt[0] == '7')
        {
          cout << "You have exited the application.\n";
          break;
        }
        else if(opt[0] == '8')
        {
          AdminMenu(sd);
        }
        clearMsg(opt);
      }
      close(sd);
    }
  }
  else if(opt[0] == '2'){
    printf("\033c");
    cout << "\nNEWSFEED\n\n";
    char message[40000];
    clearMsg(message);
    read(sd, message, 40000);
    cout << message << endl;
  }
  else if(opt[0] == '3'){
    CreateAccount(sd);
  }
  else if(opt[0] == '4'){
    cout << " DETAILS\n";
  }
  else if(opt[0] == '5'){
    cout << "You have exited the application.\n";
    close(sd);
  }
  close(sd);
}
