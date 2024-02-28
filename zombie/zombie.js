// Planet x and y dimensions are equal and maximum is set below
var MAX_PLANET_SIZE = 64;
// The actual planet size
var dimension = 0;
// Movement direction list
var directions = [];
// Creatures on the planet
var creatures = [];
// Zombies on the planet
var zombies   = [];

// List of errors
var ERROR_OK                     = 0;
var ERROR_TOO_FEW_ARGS           = 1;
var ERROR_INVALID_MATRIX_SIZE    = 2;
var ERROR_INVALID_ZOMBIE_START   = 3;
var ERROR_INVALID_CREATURE_START = 4;
var ERROR_INVALID_DIRECTION      = 5;

class Creature {
    constructor(posX, posY) {
        this.posX = posX;
        this.posY = posY;
    }
}

class Zombie extends Creature {
    constructor(posX, posY) {
        super(posX, posY);
        this.stepsDone = 0;
    }
}

class Movement {  // Looks like Creature but that is just a coincidence
    constructor(posX, posY) {
        this.posX = posX;
        this.posY = posY;
    }
}


function printUsage() {
    console.error("\nUsage:");
    console.error("------");
    console.error("node zombie.js <dimensions> <zombie position in x,y format> <creature position in one or multiple space separated x,y format(s)> <step directions in a string like UDLR>");
    console.error("Example command line:");
    console.error("node zombie.js 4 3,1 0,1 1,2 1,1 RDRU");
    console.error("Please see README for details");
}

function getCoord(argPos) {
    let tmpPos = process.argv[argPos].split(",");
    if (tmpPos.length != 2) {
        console.error(`String splitted into ${tmpPos.length} part(s) instead of 2 from string ${process.argv[argPos]}`);
        return false;
    }
    if (! /^\d+$/.test(tmpPos[0]) || ! /^\d+$/.test(tmpPos[1])) {
        console.error(`Non-numeric character found in ${process.argv[argPos]}`);
        return false;
    }
    tmpPos[0] = parseInt(tmpPos[0]);
    tmpPos[1] = parseInt(tmpPos[1]);
    if (tmpPos[0] >= dimension || tmpPos[1] >= dimension) {  // No check for < 0 since that is done by the regex already
        console.error(`Out of bounds in ${process.argv[argPos]}, valid dimensional values are: 0..${dimension-1}`);
        return false;
    }

//console.error(`x: ${tmpPos[0]}, ${tmpPos[1]}`);

    return tmpPos;
}

function setupDirections(directions) {
	let dirs = directions.toUpperCase();
	let out = [];

	for (i = 0; i < dirs.length; i++) {
	    switch (dirs.charAt(i)) {
        case 'U':
        	out[out.length] = new Movement(0, -1);
        	break;
        case 'D':
        	out[out.length] = new Movement(0, 1);
        	break;
        case 'L':
        	out[out.length] = new Movement(-1, 0);
        	break;
        case 'R':
        	out[out.length] = new Movement(1, 0);
        	break;
        default:
            console.error(`Invalid movement direction: ${directions.charAt(i)}`);
            return false;
            break;  // Just to look consistent, not that it does anything
	    }
	}
	
	return out;
}

function setInBounds(pos) {  // Just one function since planet is square x == y
    if (pos < 0) {
        pos = dimension - 1;
    }
    if (pos >= dimension) { // It really never should be gt, just ==
        pos = 0;
    }
    return pos;
}

function getFormattedOutput(arr) {
	let posString = "";
	for (i = 0; i < arr.length; i++) {
	    posString += "(" + arr[i].posX + "," + arr[i].posY + ")";
 	   if (i + 1 < arr.length) {
	        posString += " ";
	    }
	}
	return posString;
}

// main


// Sanity check
if (process.argv.length < 6) {
    console.error(`Error: less than minimum number of required command line arguments received: (${process.argv.length})`);
    printUsage();
    return ERROR_TOO_FEW_ARGS;
} 

// Dimension check
if (! /^\d+$/.test(process.argv[2])) {
    console.error('Dimension contains non-numeric character or arithmetical sign. Do not use + (nor -) prefix), neither floating numbers');
    printUsage();
    return ERROR_INVALID_MATRIX_SIZE;
}

dimension = process.argv[2];
if (dimension < 1 || dimension > MAX_PLANET_SIZE) {
    console.error(`Planet size must be between 1 and ${MAX_PLANET_SIZE} (found: ${process.argv[2]})`);
    printUsage();
    return ERROR_INVALID_MATRIX_SIZE;
}

// Zombie setup
tmpPos = getCoord(3);
if (tmpPos === false) {
    console.error('Check initial Zombie location');
    printUsage();
    return ERROR_INVALID_ZOMBIE_START;
}
zombies[0] = new Zombie(tmpPos[0], tmpPos[1]);

// Creatures setup
for (i = 4; i < process.argv.length - 1; i++) {
    tmpPos = getCoord(i);
    if (tmpPos === false) {
        console.error(`Check creature location, creature number: ${i-3}, arg number: ${i-1}`);
        printUsage();
        return ERROR_INVALID_CREATURE_START;
    }
    // Make sure that creature does not stomp on a zombie
    if (zombies[0].posX == tmpPos[0] && zombies[0].posY == tmpPos[1]) {
        console.error(`A creature is positioned on the starting zombie at ${tmpPos[0]}, ${tmpPos[1]}. That is not a good idea`);
        printUsage();
        return ERROR_INVALID_CREATURE_START;
    }
    // Make sure that the entry is uniqe
    for (j = 0; j < creatures.length; j++) {
        if (creatures[j].posX == tmpPos[0] && creatures[j].posY == tmpPos[1]) {
        	console.error(`More than one creatures are positioned at ${tmpPos[0]}, ${tmpPos[1]}. Do not do that`);
            printUsage();
            return ERROR_INVALID_CREATURE_START;	
        }
    }
    creatures[creatures.length] = new Creature(tmpPos[0], tmpPos[1]);
}

// Moving directions setup
directions = setupDirections(process.argv[process.argv.length - 1]);
if (directions === false) {
    printUsage();
    return ERROR_INVALID_DIRECTION;
}

// Setup should be okay here, lets go with the steps
let unfinished = 0;
while (unfinished < zombies.length) {  // There is any zombie to move
    let movingZombie = unfinished;     // Start moving from
    while (movingZombie < zombies.length) {  // Since new zombies can be added on the fly and then move
        let newPosX = zombies[movingZombie].posX + directions[zombies[movingZombie].stepsDone].posX;
        let newPosY = zombies[movingZombie].posY + directions[zombies[movingZombie].stepsDone].posY;
        newPosX = setInBounds(newPosX);
        newPosY = setInBounds(newPosY);
        zombies[movingZombie].posX = newPosX;
        zombies[movingZombie].posY = newPosY;
        console.log(`zombie ${movingZombie} moved to (${newPosX},${newPosY}).`);  // With end of line dot
        
        // Now check if any creature turns to be a zombie
        for (i = 0; i < creatures.length; i++) {
            if (creatures[i].posX == newPosX && creatures[i].posY == newPosY) {
            	// Add zombie, remove creature
                zombies[zombies.length] = new Zombie(newPosX, newPosY);
                creatures.splice(i, 1);
                console.log(`zombie ${movingZombie} infected creature at (${newPosX},${newPosY})`);  // Without end of line dot
                break; // No two creatures are allowed on the same place
            }
        }

        zombies[movingZombie].stepsDone++;
        if (zombies[movingZombie].stepsDone == directions.length) {
            unfinished++;  // This zombie has no more move left
        }

    	movingZombie++;
    }
}

// Report results
console.log("zombies’ positions:");
console.log(getFormattedOutput(zombies));
console.table(zombies);


console.log("creatures’ positions:");
if (creatures.length == 0) {
    console.log("none");
} else {
	console.log(getFormattedOutput(creatures));
	console.table(creatures);
}


return ERROR_OK;
