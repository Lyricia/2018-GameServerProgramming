myid = 999999;
x = -1;
y = -1;
originhp = 100;
hp = 100;
level = 1;

-- 01 player
-- 10 pm 15 pr 20 cm 25 cr 50 boss

type = -1;

attackrange = 1;
attackdamage = 10;

targetid = 999999;

MoveCounter = 3;
Activation = false;

UP = 0;
DOWN = 1;
LEFT = 2;
RIGHT = 3;
dist = -1;

aggressive = false;
counter = 10;

function setrenew()
	targetid = 999999;
	Activation = false;
	MoveCounter = 3;
	hp = originhp;
	if (type == 10) then
		attackrange = 1;
	elseif (type == 15) then
		attackrange = 3;
	elseif (type == 20) then
		attackrange = 1;
		aggressive = true;
	elseif (type == 25) then
		attackrange = 3;
		aggressive = true;
	elseif (type == 50) then
		attackrange = 2;
		aggressive = true;
	end
end

function setPosition(inputX, inputY)
	x = inputX;
	y = inputY;
end

function set_myid(x)
	myid = x;
end

function goodbyePlayer()
	API_sendMessage(targetid, myid, "Bye!!");
	Activation = false;
	targetid = 999999;
end

function setStatus(_hp, _type, _lvl)
	hp = _hp;
	originhp = hp;
	type = _type;
	level = _lvl;

	hp = 100 * level;
	attackdamage = 10 * level;
	if (type == 10) then
		attackrange = 1;
	elseif (type == 15) then
		attackrange = 3;
	elseif (type == 20) then
		attackrange = 1;
		aggressive = true;
	elseif (type == 25) then
		attackrange = 3;
		aggressive = true;
	elseif (type == 50) then
		attackrange = 2;
		aggressive = true;
	end
end

function makedirection(player_x, player_y)
	if (x > player_x) then
		dir = LEFT;
	elseif (x < player_x) then
		dir = RIGHT;
	elseif (y > player_y) then
		dir = DOWN;
	elseif (y < player_y) then
		dir = UP;
	elseif (x == player_x) then
		if (y == player_y) then
			dir = -2;
		end
	end
	return dir;
end

function Event_MoveNPC()
	MoveCounter = MoveCounter - 1;
	isActive = API_get_playeractive(targetid);
	if (isActive == 0) then
		goodbyePlayer()
		do return end;
	end

	player_x = API_get_x(targetid);
	player_y = API_get_y(targetid);
	dir = -1;
	
	if (type == 20 or type == 25 or type == 50) then
		dir = makedirection(player_x, player_y);
	elseif (type == 10 or type == 15) then
		dir = math.random(4);
	end
	
	dist = getDist(x, y, player_x, player_y);

	viewrange = type/10 + 5;
	
	if (dist <= viewrange*viewrange) then
		MoveCounter = 3;
		if(dist <= attackrange * attackrange and aggressive == true) then
			API_Attack_Player(myid, targetid, attackdamage);
			dir = -2;
		end
		if(type == 50) then
			counter = counter - 1;
			if(counter == 0) then
				counter = 10;
				API_Attack_Player(myid, targetid, -1);
			elseif (counter == 3) then
				API_Attack_Player(myid, targetid, -2);
			end
			if (counter<=3) then
				dir = -2;
			end
		end
	else
		MoveCounter = 0;
		counter = 10;
		goodbyePlayer()
		do return end;
	end

	if (MoveCounter > 0) then
		API_MoveNPC(myid, dir);
	else
		goodbyePlayer()
	end
end

function ThrowFireBall(player_x, player_y)
	
end

function getDist(ax, ay, bx, by)
	dist = (ax - bx) * (ax - bx) + (ay - by) * (ay - by); 
	return dist;
end

function IsDead()
	return hp <= 0;
end

function event_move(player)
	player_x = API_get_x(player);
	player_y = API_get_y(player);
	
	dist = getDist(x, y, player_x, player_y);
	
	if (dist <= 5*5) then
		targetid = player;
		MoveCounter = 4;
		
		if (Activation == false) then
			MoveCounter = 3;
			API_sendMessage(player, myid, "Hello!");
			Event_MoveNPC();
		end
		
		Activation = true;
	else
		targetid = 999999;
	end
end

function event_get_Damaged(damage, player)
	if (targetid == 999999) then
		targetid = player;
	end
	
	aggressive = true;
	hp = hp - damage;
	if (hp <= 0) then
		hp = 0;
	end
	return hp;
end

function test()
	print("move test\n");
end