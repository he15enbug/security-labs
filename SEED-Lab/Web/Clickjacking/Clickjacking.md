# Clickjacking
- clickjacking, also known as a "UI redress attack", is an attack that tricks a user into clicking on something they do not intend to when visiting a webpage, thus "hijacking" the click
- in this lab, we will explore a common attack vector for clickjacking: the attacker creates a webpage that loads the content of a legitimate page but overlays one or more of its button with invisible button(s) that trigger malicious actions. When a user attempts to click on the legitimate page's buttons, the browser registers a click on the invisible button instead, triggering the malicious action

## Task 1: Copy the site!
- modify `attacker.html` so that it mimics the Alice's Cupcakes website as closely as possible
- use `iframe` (inline frame) to embed Alice's website within attacker's website: `<iframe src="http://www.cjlab.com"></iframe>`
    - also modify the `attacker.css`
        ```
        iframe {
            position: absolute;
            border: none;
            width: 100%;
            height: 100%;
        }
        ```
## Task 2: Let's Get Clickjacking!
- basic clickjacking attack: add code to the CSS specification of a "button" object in `attacker.css` to make the malicious button in `attacker.html` invisible, and position the button so that it covers the "Explore Menu" button within the iframe

- by adjusting `margin-left`, `margin-top`, `color`, and `background-color`
    ```
    button {
        ...
        color: rgba(255, 255, 255, 0);
        background-color: rgba(255, 255, 255, 0);
        margin-top: 19%;
        margin-left: 2.5%;
    }
    ```
- if we click "Explore Menu", we will jump to `hacked.html`, with a hint: "You Have Been Hacked!!"

## Task 3: Bust That Frame!
- `Frame busting` is the practice of preventing a web page from being displayed within a frame, thus defending against the type of attack implemented in the previous Task
- one way to bust frames is to include script code in the webpage source that prevents the site from being framed (i.e., it prevents other site from opening the webpage in an iframe)
- make the defender's page always the topmost window on any page where it is displayed, thus preventing buttons on an attacker's page from being overlaid on top of it
- open `Labsetup/defender/index.html`, add code to `makeThisFrameOnTop()`
    ```
    function makeThisFrameOnTop() {
        if(window.self != window.top) {
            window.top.location = window.self.location;
        }
    }
    ```
- if we open `www.cjlab-attacker.com`, when the page is loaded, the top window will be reset to `www.cjlab.com` (but in some browsers, redirect will be blocked)

## Task 4: Attacker Countermeasure (Bust the Buster)
- create a workaround for frontend clickjacking defenses like frame busting
- the simplest in the current scenario is to add the `sandbox` attribute to the malicious frame
    - `<iframe src="http://www.cjlab.com" sandbox=""></iframe>`
- the `sandbox` attribute in an iframe provides a restricted environment for the content within the iframe, it can disable all scripts inside this iframe, so the previous countermeasure will not work any more

## Task 5: The Ultimate Bust
- frame busting is not sufficient to prevent clickjacking, we need back-end defenses, modern websites can cooperate with common browsers to provide such defenses
- the previous attacks all rely on the ability of an attacker's webpage code (running in a victim's browser) to fetch a benign website's content before the benigh webpage code has a chance to execute any front-end defenses
- to block this capability, special HTTP headers have been created that specify to browsers the circumstances under which a website's content should or should not be loaded
- One header is `X-Frame-Options`, and a newer, more popular one is `Content-Security-Policy` (this is also used to prevent XSS attack)
- the CSP header directive relevant for preventing clickjacking is `frame-ancestors`, which specifies the valid parents that may embed a page in a frame
- modify the defender's response headers: open Apache configuration file for the defender's website `Labsetup/image_defender/apache_defender.conf`, and uncomment the lines that specify the HTTP response headers served with the page, and substitute appropriate text in order to prevent the clickjacking attacks
    ```
    Header set Content-Security-Policy " \
                frame-ancestors 'none'; \
            "
    Header set X-Frame-Options "DENY"
    ```
- try these 2 headers one at a time, rebuild and start the containers, then visit `www.cjlab-attacker.com`, in both cases, the benign page won't be loaded, and there will be an error information: "To protect your security, www.cjlab.com will not allow Firefox to display the page if another site has embedded it. To see this page, you need to open it in a new window"

- `X-Frame-Option`:
    - `DENY` the page cannot be displayed in a frame
    - `SAMEORIGIN` the page can only be displayed ina frame on the same origin as the page itself
- `Content-Security-Policy: frame-ancestors`: specifies valid sources for embedding the resource using frame, iframe, object, embed, or applet tags. It restricts the URLs that can be used as the parent document of the current document
