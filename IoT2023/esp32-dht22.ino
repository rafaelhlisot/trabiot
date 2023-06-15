/* 
 Para testar a simulação, abra o endereço http://localhost:9080
 O gateway IoT requer a assinatura do clube Wokwi
*/
#include "SSD1306.h"
#include <driver/adc.h>
SSD1306 display(0x3c, 5, 4);
#include <WiFi.h>
#include <WebServer.h>
const char* ssid = "Wokwi-GUEST";
const char* password = "";
WebServer server(80);
#include "DHTesp.h"
#define DHTPIN 13
DHTesp dht;
float temperatura;
float umidade;
char stringTemperatura[7];
char stringUmidade[7];
unsigned long lastMsg = 0;
int intervalo_medicao = 2000;
unsigned long now;
int pinLED = 2;
bool statusLED = LOW;
void mostraTexto(int col, int lin, int tam, String txt) {
 if (tam==0) {
 display.setFont(ArialMT_Plain_10); 
 } else if (tam==1) {
 display.setFont(ArialMT_Plain_16); 
 } else {
 display.setFont(ArialMT_Plain_24); 
 }
 display.drawString(col, lin, txt);
}
void mostraDados(float temperatura, int umidade, boolean mostraUmidade=true) {
 Serial.printf("Temperatura: %.2f °C\n", temperatura);
 if (mostraUmidade) {
 Serial.printf("Umidade: %d%%\n", umidade); 
 }
 Serial.println();
 
 display.clear();
 mostraTexto( 0, 0, 1, "Temp.: " + (String)temperatura + " °C");
 if (mostraUmidade) {
 mostraTexto( 0, 25, 1, "Umid.: " + (String)umidade + "%");
 }
 mostraTexto(15, 50, 0, "Cloud & IoT - Unoesc");
 display.display();
}
void medirTemperaturaUmidade() {
 TempAndHumidity data = dht.getTempAndHumidity();
 // Sensor DHT ou nenhum sensor
 float temperatura = data.temperature;
 float umidade = data.humidity;
 if (isnan(temperatura) || isnan(umidade)) {
 Serial.println("Erro de leitura");
 temperatura = random(0, 45);
 umidade = random(0, 100);
 }
 mostraDados(temperatura, umidade);
 dtostrf(temperatura, 5, 0, stringTemperatura);
 dtostrf(umidade, 5, 0, stringUmidade);
}
void manipula_sensor() {
 medirTemperaturaUmidade();
 String strJson = "{\"temperatura\": ";
 strJson += stringTemperatura;
 strJson += ", \"umidade\":";
 strJson += stringUmidade;
 strJson += "}";
 server.send(200, "application/json", strJson);
}
void enviaHTML(bool statusLED) {
 Serial.print("Estado do GPIO2 (LED): ");
 Serial.println(statusLED ? "Ligado" : "Desligado");
 digitalWrite(pinLED, statusLED);
 
 server.send(200, "text/html", montaHTML(statusLED));
}
void handle_OnConnect() {
 statusLED = LOW;
 medirTemperaturaUmidade();
 Serial.println("Novo cliente conectado");
 
 enviaHTML(statusLED);
}
void handle_ledon() {
 statusLED = HIGH;
 enviaHTML(statusLED);
}
void handle_ledoff() {
 statusLED = LOW;
 enviaHTML(statusLED);
}
void handle_NotFound() {
 server.send(404, "text/plain", "Página não encontrada");
}
String montaHTML(bool statusLED) {
 String pagina = "<!DOCTYPE html> <html>\n";
 pagina += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, userscalable=no\">\n";
 pagina += "<link href=\"https://fonts.googleapis.com/css?family=Open+Sans:300,400,600\ rel=\"stylesheet\">\n";
 pagina += "<title>IoT com ESP32</title>\n";
 pagina += "<style>html { font-family: 'Open Sans', sans-serif; display: block; margin: 0px auto; text-align: center;color: #333333;}\n";
 pagina += "body{margin-top: 50px;}\n";
 pagina += "h1 {margin: 50px auto 30px;}\n";
 pagina += ".side-by-side{display: inline-block;vertical-align: middle;position: relative;}\n";
 pagina += ".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
 pagina += ".button-on {background-color: #3498db;}\n";
 pagina += ".button-on:active {background-color: #2980b9;}\n";
 pagina += ".button-off {background-color: #34495e;}\n";
 pagina += ".button-off:active {background-color: #2c3e50;}\n";
 pagina += ".humidity-icon{background-color: #3498db;width: 30px;height: 30px;border-radius: 50%;line-height: 36px;}\n";
 pagina += ".humidity-text{font-weight: 600;padding-left: 15px;font-size: 19px;width: 160px;text-align: left;}\n";
 pagina += ".humidity{font-weight: 300;font-size: 60px;color: #3498db;}\n";
 pagina += ".temperature-icon{background-color: #f39c12;width: 30px;height: 30px;border-radius: 50%;lineheight: 40px;}\n";
 pagina += ".temperature-text{font-weight: 600;padding-left: 15px;font-size: 19px;width: 160px;text-align: left;}\n";
 pagina += ".temperature{font-weight: 300;font-size: 60px;color: #f39c12;}\n";
 pagina += ".superscript{font-size: 17px;font-weight: 600;position: absolute;right: -20px;top: 15px;}\n";
 pagina += ".data{padding: 10px;}\n";
 pagina += "</style>\n";
 pagina += "<script>\n";
 pagina += "setInterval(getDadosSensor, 500);\n";
 pagina += "function getDadosSensor() {\n";
 pagina += "var xhttp = new XMLHttpRequest();\n";
 pagina += "xhttp.onreadystatechange = function() {\n";
 pagina += "if (this.readyState == 4 && this.status == 200) {\n";
 pagina += "var jsonResponse = JSON.parse(this.responseText);\n";
 pagina += "document.getElementById(\"valorTemperatura\").innerHTML = jsonResponse.temperatura;\n";
 pagina += "document.getElementById(\"valorUmidade\").innerHTML = jsonResponse.umidade;\n";
 pagina += "}\n";
 pagina += "};\n";
 pagina += "xhttp.open(\"GET\", \"lesensor\", true);\n";
 pagina += "xhttp.send();\n";
 pagina += "}\n";
 pagina += "</script>\n";
 pagina += "</head>\n";
 pagina += "<body>\n";
 pagina += "<div id=\"webpage\">\n";
 pagina += "<h1>Servidor Web ESP32</h1>\n";
 pagina += "<h2>Sensor DHT</h2>\n";
 pagina += "<div class=\"data\">\n";
 pagina += "<div class=\"side-by-side temperature-icon\">\n";
 pagina += "<svg version=\"1.1\" id=\"Layer_1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n";
 pagina += "width=\"9.915px\" height=\"22px\" viewBox=\"0 0 9.915 22\" enable-background=\"new 0 0 9.91522\" xml:space=\"preserve\">\n";
 pagina += "<path fill=\"#FFFFFF\" d=\"M3.498,0.53c0.377-0.331,0.877-0.501,1.374-0.527C5.697-0.04,6.522,0.421,6.924,1.142\n";
 pagina += "c0.237,0.399,0.315,0.871,0.311,1.33C7.229,5.856,7.245,9.24,7.227,12.625c1.019,0.539,1.855,1.424,2.301,2.491\n";
 pagina += "c0.491,1.163,0.518,2.514,0.062,3.693c-0.414,1.102-1.24,2.038-2.276,2.594c-1.056,0.583-2.331,0.743-3.501,0.463\n";
 pagina += "c-1.417-0.323-2.659-1.314-3.3-2.617C0.014,18.26-0.115,17.104,0.1,16.022c0.296-1.443,1.274-2.717,2.58-3.394\n";
 pagina += "c0.013-3.44,0-6.881,0.007-10.322C2.674,1.634,2.974,0.955,3.498,0.53z\"/>\n";
 pagina += "</svg>\n";
 pagina += "</div>\n";
 pagina += "<div class=\"side-by-side temperature-text\">Temperatura</div>\n";
 pagina += "<div class=\"side-by-side temperature\">";
 pagina += "<span id=\"valorTemperatura\">";
 pagina += stringTemperatura;
 pagina += "</span>";
 pagina += "<span class=\"superscript\">&deg;C</span></div>\n";
 pagina += "</div>\n";
 pagina += "<div class=\"data\">\n";
 pagina += "<div class=\"side-by-side humidity-icon\">\n";
 pagina += "<svg version=\"1.1\" id=\"Layer_2\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n";
 pagina += "width=\"12px\" height=\"17.955px\" viewBox=\"0 0 13 17.955\" enable-background=\"new 0 0 1317.955\" xml:space=\"preserve\">\n";
 pagina += "<path fill=\"#FFFFFF\"d=\"M1.819,6.217C3.139,4.064,6.5,0,6.5,0s3.363,4.064,4.681,6.217c1.793,2.926,2.133,5.05,1.571,7.057\n";
 pagina += "c-0.438,1.574-2.264,4.681-6.252,4.681c-3.988,0-5.813-3.107-6.252-4.681C0.313,11.267,0.026,9.143,1.819,6.217\"></path>\n";
 pagina += "</svg>\n";
 pagina += "</div>\n";
 pagina += "<div class=\"side-by-side humidity-text\">Umidade</div>\n";
 pagina += "<div class=\"side-by-side humidity\">";
 pagina += "<span id=\"valorUmidade\">";
 pagina += stringUmidade;
 pagina += "</span>";
 pagina += "<span class=\"superscript\">%</span></div>\n";
 pagina += "</div>\n";
 if (statusLED) {
 pagina += "<p>Estado do LED: Ligado</p><a class=\"button button-off\" href=\"/ledoff\">OFF</a>\n";
 } else {
 pagina += "<p>Estado do LED: Desligado</p><a class=\"button button-on\" href=\"/ledon\">ON</a>\n";
 }
 
 pagina += "</body>\n";
 pagina += "</html>\n";
 
 return pagina;
}
void setup() {
 Serial.begin(115200);
 pinMode(pinLED, OUTPUT);
 
 display.init();
 display.clear();
 display.flipScreenVertically();
 
 Serial.println();
 Serial.print("Conectando à rede WiFi: ");
 Serial.println(ssid);
 WiFi.begin(ssid, password, 6);
 while (WiFi.status() != WL_CONNECTED) {
 Serial.print(".");
 delay(500);
 }
 Serial.println("");
 Serial.print("ESP32 conectado à rede Wi-Fi ");
 Serial.println(String(ssid) + "!");
 Serial.print("\nEndereco MAC: ");
 Serial.println(WiFi.macAddress());
 Serial.print("Endereco IP: ");
 Serial.println(WiFi.localIP());
 Serial.print("Máscara de sub-rede: ");
 Serial.println(WiFi.subnetMask());
 Serial.print("Gateway: ");
 Serial.println(WiFi.gatewayIP());
 Serial.print("DNS 1: ");
 Serial.println(WiFi.dnsIP(0));
 Serial.print("DNS 2: ");
 Serial.println(WiFi.dnsIP(1));
 server.on("/", handle_OnConnect);
 server.on("/ledon", handle_ledon);
 server.on("/ledoff", handle_ledoff);
 server.on("/lesensor", manipula_sensor);
 server.onNotFound(handle_NotFound);
 server.begin();
 Serial.println("Servidor HTTP iniciado!");
 dht.setup(DHTPIN, DHTesp::DHT22);
}
void loop() {
 now = millis();
 
 server.handleClient();
 if (now - lastMsg > intervalo_medicao) {
 lastMsg = now;
 medirTemperaturaUmidade();
 }
}