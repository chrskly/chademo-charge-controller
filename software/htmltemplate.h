
#ifndef HTML_TEMPLATE_H
#define HTML_TEMPLATE_H

#define MAIN_PAGE_BODY ""

#define QUOTE(...) #__VA_ARGS__
const char *main_page_body = QUOTE(
<html>
<head>
<style>
body {
  background-color: #512B81;
  max-width: 400px;
  margin: auto;
  font-family: Arial, Helvetica, sans-serif;
  color: white;
}
table {
    width:100%;
    background-color: #232D3F;
    color: white;
    border-collapse: collapse;
}
);

/*
td {
    padding-top: 8px;

    padding-bottom: 8px;

}

.status-container {
    border: 2px solid #232D3F;
    border-radius: 10px;
    padding: 10px;
    background-color: #232D3F;
    margin: 10px 0 0 0;
}

.status-container h1 {
    margin: 0;
    padding: 5px 0 5px 0;
    font-size: 1.2em;
    font-weight: 700;
    text-align: center;
}


.status-container p {
    padding: 2px 0 5px 0;
    margin: 0;
}
.status-container td {
    border-left: 1px solid #940B92;
    margin: auto;
    text-align: center;
    width: 50%;
}
.status-container td:first-child {
    border-left: none;
}
.status-time-container {
    float:left;
    width: 50%;
    padding: 20px 0 20px 0;
}
.status-time-container h1 {
    padding: 0;
    margin: auto;
    font-size: 0.8em;
    color: grey;
}
.status-time-container p {
    padding: 0;
    margin: auto;
    text-align: center;
    font-size: 3em;
}
.status-power-container {
    float: left;
    width: 45%;
    margin: 20px 0 20px 0;
    border-left: 1px solid grey;
}
.status-power-container h1 {
    padding: 0;
    margin:auto;
    font-size: 0.8em;
    color: grey;
}
.status-power-container p {
    padding: 0;
    margin:auto;
    text-align: center;
    font-size: 3em;
}
.progress-background {
    width:100%;
    background-color: grey;
    border: 0px solid grey;
    border-radius: 5px;
}
.progress-fill {
    background-color: #03C988;
    height:46px;
    border: 1px solid #039958;
    border-radius: 5px;
}
.progress-fill p {
    text-align: center;
    vertical-align: middle;
    line-height: 46px;
    color: white;
    font-size: 2em;
    font-weight: 700;
    text-shadow: rgba(0, 0, 0, .3) 1px 1px 1px;
}
.table-container {
    border: 2px solid #232D3F;
    border-radius: 10px;
    padding: 10px;
    background-color: #232D3F;
}
.table-container tr {
    border-top: 1px solid grey;
}
.table-container tr:first-child {
    border:none;
}
.button {
    background-color: #DA0C81;
    color: white;
    width: 100%;
    border-radius: 11px;
    border: 0;
    line-height:36px;
    font-size: 1.15em;
    font-weight: 700;
    text-shadow: rgba(0, 0, 0, .3) 1px 1px 1px;
}

const char *main_page_body_2 = QUOTE(
</style>
</head>
<body>
    <div class='status-container'>
        <h1>CHAdeMO CHARGER</h1>
        <div class='status-time-container'>
            <p>&infin; min</p>
            <h1>TIME REMAINING</h1>
        </div>
        <div class='status-power-container'>
            <p>0 kW</p>
            <h1>CHARGE POWER</h1>
        </div>
        <h1>PROGRESS</h1>
        <div class='progress-background'>
            <div class='progress-fill' style='width:1%'><p>1%</p></div>
        </div>
    </div>
    <br>
    <div class='table-container'>
        <table>
            <tr><td>Status</td><td>Energy transfer</td></tr>
            <tr><td>Current</td><td>0A</td></tr>
            <tr><td>Time elapsed</td><td>0 mins</td></tr>
            <tr><td>Max Voltage</td><td>0V</td></tr>
            <tr><td>Battery SoC</td><td>65%</td></tr>
            <tr><td>Battery temperature <br>(min / avg / max)</td><td>15°c / 16°c / 18°c</td></tr>
            <tr><td>Energy delivered</td><td>2.4kWh</td></tr>
        </table>
    </div>
    <br>
    <button class='button' role='button'>Stop charging</button>
    <p>Version %s</p>
</body>
</html>
);
*/

#endif