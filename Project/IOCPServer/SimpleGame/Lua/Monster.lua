myid = 999999;
x = -1;
y = -1;
targetid = 999999;
MoveCounter = 3;
Activation = false;

function setPosition(inputX, inputY)
	x = inputX;
	y = inputY;
end

function set_myid(x)
	myid = x;
end

function Event_MoveNPC()
	MoveCounter = MoveCounter - 1;
	if(MoveCounter > 0) then
		API_MoveNPC(myid);
	else
		API_sendMessage(targetid, myid, "Bye!!");
		Activation = false;
	end
end

function event_player_move(player)
	targetid = player;
	player_x = API_get_x(player)
	player_y = API_get_y(player)
	my_x = API_get_x(myid)
	my_y = API_get_y(myid)
	if (player_x == my_x) then
		if (player_y == my_y) then
			MoveCounter = 4;
			API_sendMessage(player, myid, "Hello  ");

			if (Activation == false) then
				MoveCounter = 3;
				API_MoveNPC(myid);
			end

			Activation = true;
		end
	end
end