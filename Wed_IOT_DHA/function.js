// khoi tao cac thong so
let btn1 = document.querySelector('#btn1');
let img = document.querySelector('#img');
let btn2 = document.querySelector('#btn2');

btn1.addEventListener('click', ()=>{
    img.src = 'denon1.png'; 
    firebase.database().ref().update({Relay1: "0"})

})

btn2.addEventListener('click', ()=>{
    img.src = 'den1.png';
    firebase.database().ref().update({Relay1: "1"})
})

// khoi tao cac thong so
let btn3 = document.querySelector('#btn3');
let img1 = document.querySelector('#img1');
let btn4 = document.querySelector('#btn4');

// functions nut bam
btn3.addEventListener('click', ()=>{
    img1.src = 'fan_running.gif'; 
    firebase.database().ref().update({Relay2: "0"})
})

btn4.addEventListener('click', ()=>{
    img1.src = 'fan_off.png';
    firebase.database().ref().update({Relay2: "1"})
})

let btn5 = document.querySelector('#btn5');
let img2 = document.querySelector('#img2');
let btn6 = document.querySelector('#btn6');

// functions nut bam
btn5.addEventListener('click', ()=>{
    img2.src = 'maybom.gif'; 
    firebase.database().ref().update({Relay3: "0"})
})

btn6.addEventListener('click', ()=>{
    img2.src = 'maybom.png';
    firebase.database().ref().update({Relay3: "1"})
})


