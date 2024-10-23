#include <stdio.h>
#include <stdlib.h>
#include "C:\Bitnami\wampstack-8.1.2-0\mariadb\include\mysql\mysql.h"
 
 
int main()
{

    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    char *server = "localhost";
    char *user = "root";
    char *password = "";
    char *database = "test_1023";
	 
	 
    conn = mysql_init(NULL);
 
    // 첫번째 인자, conn: mysql에 대한 주소값. init해준 리턴값을 가져가면 됨.
    if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0))
    {
        exit(1);
    }
    if(mysql_query(conn,"show tables"))
    {
        // query날리기. CGI programming, connect해서 들어가면 해당 프로그램 커맨드를 사용하면 됨.
        //      conn DB에서 "show tables" 명령어를 처리해라.
        exit(1);
    }
	 
    res = mysql_use_result(conn);
    printf("MYSQL Tables in mysql database : \n");

    // 결과값 print하기
    while((row = mysql_fetch_row(res)) != NULL)
        printf("%s \n",row[0]);
	 
	 
    if(mysql_query(conn,"SELECT * FROM R_TEST"))
    {
        return 1;
    }
 
    res = mysql_use_result(conn);
	 
    printf("Returning List of Names : \n");
    while((row = mysql_fetch_row(res)) != NULL)
        printf("%s %s %s \n",row[0],row[1],row[2]);
 
	 
    mysql_free_result(res);
    mysql_close(conn);
    return 0;
}



