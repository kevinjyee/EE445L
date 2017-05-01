// BACKGROUND
var bgReady = false;
var bgImage = new Image();

bgImage.onload = function(){
	bgReady = true;
};

bgImage.src = "imgs/GameBackground.jpg";

var heroReady = false;
var heroImage = new Image();

heroImage.onload = function(){
	heroReady = true;
}
heroImage.src = "imgs/hero_idle.jpg";

var monsterReady = false;
var monsterImage = new Image();

monsterImage.onload = function(){
	monsterReady = true;
}
monsterImage.src = "imgs/monster_idle.jpg";