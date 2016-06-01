<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Loading</title>

    <meta http-equiv="refresh" content="4">

    <style>

        .inner {
            position: absolute;
            box-sizing: border-box;
            width: 120%;
            height: 120%;
            border-radius: 50%;
        }

        .inner.one {
            left: -15%;
            top: 0%;
            animation: rotate-one 1s linear infinite;
            border-style: solid;
            border-color: #1D70B7;
            border-width: 3px 0px 0px 4px;
        }


        .inner.three {
            right: 0%;
            bottom: -15%;
            animation: rotate-three 1s linear infinite;
            border-style: solid;
            border-color: #5e5e62;
            border-width: 3px 4px 3px 0px;
        }

        @keyframes rotate-one {
            0% {
                transform: rotateX(35deg) rotateY(-45deg) rotateZ(0deg);
            }
            100% {
                transform: rotateX(35deg) rotateY(-45deg) rotateZ(360deg);
            }
        }


        @keyframes rotate-three {
            0% {
                transform: rotateX(45deg) rotateY(25deg) rotateZ(0deg);
            }
            100% {
                transform: rotateX(45deg) rotateY(25deg) rotateZ(360deg);
            }
        }


        @-webkit-keyframes opacity {
            0% { opacity: 1; }
            100% { opacity: 0; }
        }
        @-moz-keyframes opacity {
            0% { opacity: 1; }
            100% { opacity: 0; }
        }


        #loading span {
            -webkit-animation-name: opacity;
            -webkit-animation-duration: 1s;
            -webkit-animation-iteration-count: infinite;

            -moz-animation-name: opacity;
            -moz-animation-duration: 1s;
            -moz-animation-iteration-count: infinite;
        }

        #loading span:nth-child(2) {
            -webkit-animation-delay: 100ms;
            -moz-animation-delay: 100ms;
        }

        #loading span:nth-child(3) {
            -webkit-animation-delay: 300ms;
            -moz-animation-delay: 300ms;
        }
        @media only screen and (max-width: 600px) {
            /* For mobile phones: */

            .loader {
                width: 50px;
                height: 50px;
            }


        }


        #container {

            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            height: 80px;
            width: 390px;
        }

        #logo {
            position: relative;
            margin: 0px 100px;
        }

        .loader {
            position: absolute;
            margin: 4% auto;
            width: 55px;
            height: 55px;
            border-radius: 50%;
            perspective: 800px;
            float: left;
        }

        img{
            margin-left: 60px;
            position: absolute;

        }
    </style>
</head>
<body>

<div id="container">
    <div id="logo">
        <div class="loader">
            <div class="inner one"></div>
            <div class="inner three"></div>
        </div>
            <img src="img/loading-logo.png">
    </div>

</div>



</body>
</html>