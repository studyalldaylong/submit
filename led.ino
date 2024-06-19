#include <ESP8266WiFi.h>

// 设置连接wifi
const char* ssid = "On"; 
const char* password = "88887777"; 

// 设置web服务器端口
WiFiServer server(80);

// 定义储存HTTP请求的变量
String header;

// 定义储存当前输出状态的变量
String output5State = "off";
String output4State = "off";

// 定义输出引脚连接
const int output5 = 5; // esp8266的IO口5,既D1
const int output4 = 2; // esp8266的IO口2,既D4

void setup() {
    Serial.begin(115200);

    // 设置引脚为输出模式
    pinMode(output5, OUTPUT);
    pinMode(output4, OUTPUT);

    // 将引脚输出初始化为低
    digitalWrite(output5, LOW);
    digitalWrite(output4, LOW);

    // 通过wifi名称和密码连接网络
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    // 串口打印网站的IP地址，并启动服务器
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    server.begin();
}

void loop() {
    WiFiClient client = server.available(); // 监听是否有客户端连接
    if (client) { // 如果有新的客户端连接
        Serial.println("New Client."); // 在串口监视器上打印一条信息
        String currentLine = ""; // 创建一个字符串来保存传入的数据
        while (client.connected()) { // 当客户端保持连接时循环
            if (client.available()) { // 如果有字节可以读取
                char c = client.read(); // 读取一个字节
                Serial.write(c); // 在串口监视器上打印该字节
                header += c;
                if (c == '\n') { // 如果该字节是换行符
                    // 如果当前行为空，则表示连续收到两个换行符
                    // 这是客户端 HTTP 请求的结尾，发送响应
                    if (currentLine.length() == 0) {
                        // HTTP 头总是以响应代码开头（例如 HTTP/1.1 200 OK）
                        // 和内容类型，以便客户端知道接下来是什么，然后是一个空行
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-Type: text/html; charset=utf-8");
                        client.println("Content-type:text/html");
                        client.println("Connection: close");
                        client.println();
                        // 控制 GPIO 的开关
                        if (header.indexOf("GET /5/on") >= 0) {
                            Serial.println("GPIO 5 on");
                            output5State = "on";
                            digitalWrite(output5, HIGH);
                        } else if (header.indexOf("GET /5/off") >= 0) {
                            Serial.println("GPIO 5 off");
                            output5State = "off";
                            digitalWrite(output5, LOW);
                        } else if (header.indexOf("GET /4/on") >= 0) {
                            Serial.println("GPIO 4 on");
                            output4State = "on";
                            digitalWrite(output4, HIGH);
                        } else if (header.indexOf("GET /4/off") >= 0) {
                            Serial.println("GPIO 4 off");
                            output4State = "off";
                            digitalWrite(output4, LOW);
                        }
                        // 显示 HTML 网页
                        client.println("<!DOCTYPE html><html>");
                        client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
                        client.println("<link rel=\"icon\" href=\"data:,\">");
                        // CSS 用于设置滑动按钮的样式
                        client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
                        client.println(".switch { position: relative; display: inline-block; width: 60px; height: 34px;}");
                        client.println(".switch input { display: none;}");
                        client.println(".slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; transition: .4s;}");
                        client.println(".slider:before { position: absolute; content: \"\"; height: 26px; width: 26px; left: 4px; bottom: 4px; background-color: white; transition: .4s;}");
                        client.println("input:checked + .slider { background-color: #2196F3;}");
                        client.println("input:checked + .slider:before { transform: translateX(26px);}");
                        client.println(".slider.round { border-radius: 34px;}");
                        client.println(".slider.round:before { border-radius: 50%;}</style></head>");

                        // 网页标题
                        client.println("<body><h1> WIFI开关 </h1>");
                        // 显示当前状态，以及 GPIO 5 的滑动按钮
                        client.println("<p> 蓝灯开关 </p>");
                        client.println("<label class=\"switch\">");
                        client.print("<input type=\"checkbox\" onchange=\"toggleCheckbox(this, '5')\" ");
                        if (output5State == "off") {
                            client.print("checked");
                        }
                        client.println(">");
                        client.println("<span class=\"slider round\"></span></label>");

                        // 显示当前状态，以及 GPIO 4 的滑动按钮
                        client.println("<p>白灯开关 </p>");
                        client.println("<label class=\"switch\">");
                        client.print("<input type=\"checkbox\" onchange=\"toggleCheckbox(this, '4')\" ");
                        if (output4State == "off") {
                            client.print("checked");
                        }
                        client.println(">");
                        client.println("<span class=\"slider round\"></span></label>");

                        // JavaScript 用于处理滑动按钮的切换
                        client.println("<script>");
                        client.println("function toggleCheckbox(element, gpio) {");
                        client.println("  var xhr = new XMLHttpRequest();");
                        client.println("  if (element.checked) { xhr.open('GET', '/' + gpio + '/off', true); }");
                        client.println("  else { xhr.open('GET', '/' + gpio + '/on', true); }");
                        client.println("  xhr.send();");
                        client.println("}");
                        client.println("</script>");

                        client.println("</body></html>");
                        // HTTP 响应以另一个空行结束
                        client.println();
                        // 退出 while 循环
                        break;
                    } else { // 如果收到换行符，则清空 currentLine
                        currentLine = "";
                    }
                } else if (c != '\r') { // 如果收到的不是回车符，将其添加到 currentLine 末尾
                    currentLine += c;
                }
            }
        }
        // 清空 header 变量
        header = "";
        // 关闭连接
        client.stop();
        Serial.println("Client disconnected.");
        Serial.println("");
    }
}
