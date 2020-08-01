#ifndef LIB_NETWORK_EXT_SETUPPAGE_H
#define LIB_NETWORK_EXT_SETUPPAGE_H
#include <Arduino.h>


const char setupPageHeader[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<title>NodeWorks - Home Integrator</title>
<meta charset="UTF-8">
<meta name="author" content="Justin Ritter">
<meta name="company" content="NodeWorks">
<meta name="website" content="https://nodeworks.de">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
html, body, div, span, applet, object, iframe, h1, h2, h3, h4, h5, h6, p, blockquote, pre, a, abbr, acronym, address, big, cite, code, del, dfn, em, font, img, ins, kbd, q, s, samp, small, strike, strong, sub, sup, tt, var, b, u, i, center, dl, dt, dd, ol, ul, li, fieldset, form, label, legend, caption {
margin: 0;
padding: 0;
border: 0;
outline: 0;
font-size: 100%;
vertical-align: baseline;
background: transparent;
}
html, body {
height: 100%;
width: 100%;
}
.head {
background-color:  #00e2b2;
height: 8rem;
overflow: hidden;
}
.logo {
position: relative;
float: left;
height: 100%;
width: 0%;
display: block;
}
.header {
position: relative;
float: right;
height: 100%;
width: 100%;
display: block;
text-align: center;
padding-top: 1em;
}
.heading {
    padding-left: 2rem;
    padding-right: 2rem;
}

.heading h1 {
font-size: 2em;
font-family: sans-serif;
color: white;
text-shadow: 0 0 5px black;
}
.heading p {
font-size: 1.3em;
}
html {
    background-color: #f8f5f5;
}
.body {
height: auto;
background-color:#f8f5f5 ;
}
.wrapper {
width: 95%;
}
.border {
padding-top: .2rem;
padding-left: 2em;
padding-right: 3em;
}
.text {
font-family: "proxima-nova-soft", "Proxima Nova Soft", Helvetica, Arial, sans-serif;
color: #5e5e5e ;
font-size: 1.3em;
}
.wifiNetworks {
padding-top: .8em;
}
.networks {
width: 100%;
height: 20em;
background-color:   #cccccc;
overflow: scroll;
}
.miniWrap {
width: 20%;
float: left;
}
.margin-top {
margin-top: 3em;
}
.input {
width: 80%;
height: 2.4em;
}

.send-wrap {
    padding-top: 3em;
}
.send {
    
    background-color: #cccccc;
    height: 2.9em;
}

#wifi {
    margin-left: 1.3em;
}

.footer {
    padding-bottom: 4rem;
}
</style>
<script>
function c(object)
{
document.getElementById('ssidField').value=object.innerText|| object.textContent;
document.getElementById('passwordField').focus();
}

function showPw()
{
    var field = document.getElementById("passwordField");
    if(field.type == "password")
    {
        field.type = "text";
    }
    else
    {
        field.type = "password";
    }
}

function message()
{    
window.alert('Die eigegebenen Anmeldedaten werden geprüft')
}

</script>
</head>
)=====";

const char setupPageBodyPart1[] PROGMEM = R"=====(
<body>
<div class=head>
<div class="header">
<div class="logo">
</div>
<div class="heading">
<h1>Nodeworks Home-Integrator</h1>
<p>Nodeworks by Justin Ritter</p>
</div>
</div>
</div>
<div class="body">
<div class="wrapper">
<div class="content border">
<div class="wifiNetworks">
<p class="text">Zuerst benötigen wir Ihre WiFi-SSID (Netzwerkname)</p>
</div>
<div class="networks margin-top">
<nav id="wifi">
)=====";

const char setupPageBodyPart2[] PROGMEM = R"=====(
</nav>
</div>
<div class="margin-top">
<div>
<label>SSID:</label>
</div>
<div>
<form action="/cred_save" method="POST">
<input id="ssidField" class="input" type="text" name="ssid">
</div>
</div>
<br>
<div class="psk">
<div>
<label>Passwort</label>
</div>
<div>
<input id="passwordField" class="input" type="password" name="psk">

<br/>
<input type="checkbox" onclick="showPw()">Passwort zeigen
</div> 
</div>
<div class="send-wrap">
    <input onclick="message()" class ="input send" value="Speichern" type="submit">
</div>
</div>
</div>
</div>
</form>
<div class="footer">
</div>
</body>
</html>

)=====";
#endif