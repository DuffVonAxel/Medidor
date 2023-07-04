var websocket = new WebSocket('ws://'+location.hostname+'/');					// Cria uma nova instancia do WebSocket.
var meter1 = 0;																	// Variavel global para o medidor.
var meter2 = 0;																	// Variavel global para o medidor.

function sendText(name) 														// Subrotina para enviar o texto(JSON).
{
	console.log('Envia Texto.');                        						// Para debug no navegador.
	var data = {};																// Variavel local para salvar os dados.
	data["id"] = name;															// Recupera o valor para 'Id'.
	console.log('Dado=', data);                         						// Para debug no navegador.
	json_data = JSON.stringify(data);											// Formata o dado.
	console.log('json_data=' + json_data);              						// Para debug no navegador.
	websocket.send(json_data);													// Envia o texto formatado em JSON.
}

websocket.onopen = function(evt)  												// Subrotina do WebSocket: Aberto.
{
	console.log('Conexao WebSocket: Aberta.');          						// Para debug no navegador.
	var data = {};																// Variavel local para salvar os dados.
	data["id"] = "init";														// Salva com o valor 'init' para 'Id'.
	console.log('Dado=', data);                         						// Para debug no navegador.
	json_data = JSON.stringify(data);											// Formata o dado.
	console.log('json_data=' + json_data);              						// Para debug no navegador.
	websocket.send(json_data);													// Envia o texto formatado em JSON.
}

websocket.onmessage = function(evt)   											// Subrotina do WebSocket: Envia/Recebe Mensagens.
{
	var tmpV = 0.001;															// Variavel local com valor definido (esse valor evita o bug do font).
	var tmpI = 0.001;															// Variavel local com valor definido (esse valor evita o bug do font).
	var msg = evt.data;															// Variavel contem o dado do evento ocorrido.
	console.log("Msg=" + msg);                          						// Para debug no navegador.
	var values = msg.split('\x04');                     						// \x04 eh EOT(Separador de 'evento').
	switch(values[0]) 															// Verifica qual o 'evento' ocorreu...
	{
		case 'HEAD':															// ...evento chegou: 'HEAD'(cabecalho).
			console.log("HEAD values[1]=" + values[1]); 						// Para debug no navegador.
			var h1 = document.getElementById( 'header' );						// Salva 'Id'(Identificacao) do 'header'(Cabecalho) da pagina html.
			h1.textContent = values[1];											// Envia a pagina html o valor do evento.
			break;																// Interrompe o 'switch'.
		case 'METER':															// ...evento chegou: 'METER'(Medidores).
			console.log("METER values[1]="+values[1]);  						// Para debug no navegador.
			console.log("METER values[2]="+values[2]);  						// Para debug no navegador.
			if (values[1] != "") 												// Se conter um valor...
			{
				gaugeHpa.options.units = values[1];								// ... envia ao medidor a unidade.
				// document.getElementById("hpa").style.display = "inline-block";	// Faz alinhamento 'em linha' (vertical ou horizontal).
				meter1 = 1;														// Habilita o medidor.
			}
			if (values[2] != "")  												// Se conter um valor...
            {
				gaugeHpb.options.units = values[2];								// ... envia ao medidor a unidade.
				// document.getElementById("hpb").style.display = "inline-block";	// Faz alinhamento 'em linha' (vertical ou horizontal).
				meter2 = 1;														// Habilita o medidor.
			}
			break;																// Interrompe o 'switch'.
		case 'DATA':															// ...evento chegou: 'DATA'(Valores).
			console.log("DATA values[1]=" + values[1]);  						// Para debug no navegador.
			var voltage1 = parseInt(values[1], 10);								// Salva o valor do evento.
			if(voltage1>=0 && voltage1<1657)									// Se for a escala negativa... (1657=> (3172mV+143mV)/2)
			{
				tmpV = voltage1-1658;											// Calcula o valor em funcao do 'zero' central.
				tmpV *= 0.015841584; 											// Adequa a escala do medidor. =48/3030=0.015841584
			} else if(voltage1>=1657 && voltage1<=3172)							// Se for a escala positiva...
			{
				tmpV = voltage1-1657;											// Calcula o valor em funcao do 'zero' central.
				tmpV *= 0.015841584; 											// Adequa a escala do medidor. =48/3030=0.015841584
			} 
			gaugeHpa.value = tmpV;												// Atualiza o valor no medidor.
			// gaugeHpa.update({ valueText: values[1] });							// Mostra o valor(mV) da entrada no painel digital.
			if (meter2) 														// Se o medidor foi habilitado...
            {
				console.log("DATA values[2]=" + values[2]);  					// Para debug no navegador.
				var voltage2 = parseInt(values[2], 10);							// Salva o valor do evento.
				if(voltage2>=0 && voltage2<1657)								// Se for a escala negativa...
				{
					tmpI = voltage2 - 1658;										// Calcula o valor em funcao do 'zero' central.
					tmpI *= 0.001980198; 										// Adequa a escala do medidor. =6/3030=0.001980198
				} else if(voltage2>=1657 && voltage2<=3172)						// Se for a escala positiva...
				{
					tmpI = voltage2-1657;										// Calcula o valor em funcao do 'zero' central.
					tmpI *= 0.001980198;										// Adequa a escala do medidor. =6/3030=0.001980198
				} 
				gaugeHpb.value = tmpI;											// Atualiza o valor no medidor.
				// gaugeHpb.update({ valueText: values[2] });						// Mostra o valor(mV) da entrada no painel digital.
			}
			break;																// Interrompe o 'switch'.
		default:																// Se for uma situacao inesperada...
			break;																// ...interrompe o 'switch'.
	}
}

websocket.onclose = function(evt) 												// Subrotina do WebSocket: Fechado.
{
	console.log('Conexao do WebSocket fechada.');                          		// Para debug no navegador.
}

websocket.onerror = function(evt)  												// Subrotina do WebSocket: Erro.
{
	console.log('Websocket, Erro: ' + evt);                          			// Para debug no navegador.
}
