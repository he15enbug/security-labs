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
