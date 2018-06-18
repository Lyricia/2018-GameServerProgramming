myid = 999999;
x = -1;
y = -1;
hp = 100;
level = 1;

-- 01 player
-- 10 pm 15 pr 20 cm 25 cr 30 boss

type = -1;

attackrange = 1;
attackdamage = 10;

targetid = 999999;

MoveCounter = 3;
Activation = false;

UP = 0
DOWN = 1
LEFT = 2
RIGHT = 3


function setPosition(inputX, inputY)
	x = inputX;
	y = inputY;
end

function set_myid(x)
	myid = x;
end

function setStatus(_hp, _type, _lvl)
	hp = _hp;
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
	elseif (type == 25) then
		attackrange = 3;
	elseif (type == 30) then
	end
end

function Event_MoveNPC()
	MoveCounter = MoveCounter - 1;
	isActive = API_get_playeractive(targetid);
	if (isActive == 0) then
		targetid = 999999;
		do return end;
	end

	player_x = API_get_x(targetid);
	player_y = API_get_y(targetid);
	dir = -1;

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

	dist = getDist(x, y, player_x, player_y);
	if (dist <= 9) then
		MoveCounter = 3;
		if(dist <= attackrange * attackrange) then
			API_Attack_Player(myid, targetid, attackdamage);
			dir = -2;
		end
	end

	if (MoveCounter > 0) then
		API_MoveNPC(myid, dir);
	else
		API_sendMessage(targetid, myid, "Bye!!");
		Activation = false;
	end
end

function getDist(ax, ay, bx, by)
	dist = 
		(ax - bx) * (ax - bx) +
		(ay - by) * (ay - by); 
	return dist;
end

function event_player_move(player)
	player_x = API_get_x(player);
	player_y = API_get_y(player);

	dist = getDist(x, y, player_x, player_y);

	if(dist <= 9) then
		targetid = player;
		MoveCounter = 4;
		
		if (Activation == false) then
			MoveCounter = 3;
			API_sendMessage(player, myid, "Hello  ");
			Event_MoveNPC();
			--API_MoveNPC(myid);
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
	
	hp = hp - damage;
end
