'use strict';
var leftfoot = document.querySelector('.leftfoot1:nth-child(1)'),
    rightfoot = document.querySelector('.right:nth-child(2)'),

    start = 0,
    step = 20,
    isFlipped = false,
    //unicornLength = unicorn.clientWidth;

window.addEventListener('keydown', event => {
    var code = event.keyCode;

if (code === 39 || code === 37) {
    isFlipped = code === 37;

    start += code === 39 ? step : -step;

    if (start < 0) {
        start = 0;
    } else if (start +5 > window.innerWidth) {
        start = window.innerWidth - 5;
    }


   leftfoot.style.transform = `translateX(${start}px)${isFlipped ? ' rotateY(180deg)' : ''}`;
   rightfoot.style.transform = `translateX(${start}px)${isFlipped ? ' rotateY(180deg)' : ''}`;
}

if (code === 32) {
   // unicorn.style.transform = `translateX(${start}px)${isFlipped ? ' rotateY(180deg)' : ''} rotateZ(-30deg)`;
      leftfoot.style.transform = `translateX(${start}px)${isFlipped ? ' rotateY(180deg)' : ''}rotateZ(-30deg);`
   rightfoot.style.transform = `translateX(${start}px)${isFlipped ? ' rotateY(180deg)' : ''}rotateZ(-30deg)`;
}
});

//cake.addEventListener('animationend', function() {
   // var music = document.createElement('iframe');
    //music.setAttribute('hidden', true);
    //music.setAttribute('src', 'https://www.youtube.com/embed/Qwscb3QIVSg?rel=0&amp;showinfo=0&amp;autoplay=1');
    //document.body.appendChild(music);
    //window.location.href = "birthdaycard.html"
//});