-- mysql -u root -pbitnami < gsp_241023.sql

-- create database
select '' as ' ';
create database  if not exists test_1023;
show databases;

-- create table
select '' as ' ';
use test_1023;
drop table if exists R_TEST;
create table R_TEST(num int, name char(20), tel char(20));

-- show table
select '' as ' ';
show tables;
describe R_TEST;

-- insert datas into table
select '' as ' ';
insert into R_TEST values(1, 'a', 'aa');
insert into R_TEST values(2, 'b', 'bb');
insert into R_TEST values(3, 'c', 'cc');
select * from R_TEST;

-- 2024.10.23 게임서버실습(2)
-- C++에서 데이터베이스 연동하는 법

-- mysql헤더를 불러와야한다.
    -- code: #include "/usr/local/mysql/include/mysql.h"