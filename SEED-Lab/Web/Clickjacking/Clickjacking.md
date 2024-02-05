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
- if we click "Explore More", we will jump to `hacked.html`, with a hint: "You Have Been Hacked!!"

## Task 3: Bust That Frame!