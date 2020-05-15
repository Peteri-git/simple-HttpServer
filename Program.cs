using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.IO.Ports;
using TPLinkSmartDevices.Devices;

namespace HttpServer
{
    class Program
    {
        delegate void SimpleDelegate();
        static SerialPort port = new SerialPort("COM4",115200, Parity.None, 8, StopBits.One);
        static Dictionary<string, SimpleDelegate> dic = new Dictionary<string, SimpleDelegate>();
        static string path;
        static TPLinkSmartBulb bulb = new TPLinkSmartBulb("192.168.1.5");
        static void Main(string[] args)
        {
            Console.WriteLine("Zadejte IP");
            string ip = Console.ReadLine();
            Console.WriteLine("Zadejte Port");
            string port = Console.ReadLine();
            Console.WriteLine("Zadej cestu k složce");
            path = Console.ReadLine();
            Console.WriteLine();
            dic.Add("/On", new SimpleDelegate(On));
            dic.Add("/Blinking", Blinking);
            dic.Add("/Off", new SimpleDelegate(Off));
            dic.Add("/Switch",new SimpleDelegate(Switch));
            TcpListener server = new TcpListener(IPAddress.Parse(ip),Convert.ToInt32(port));
            server.Start();
            while (true)
            {
                var client = server.AcceptTcpClient();
                Thread th = new Thread(process);
                th.Start(client);
            }
        }
        static void On()
        {
            bulb.PoweredOn = true;
        }
        static void Off()
        {
            bulb.PoweredOn = false;
        }

        static void Switch()
        {
            if (bulb.PoweredOn == true)
            {
                bulb.PoweredOn = false;
            }
            else if (bulb.PoweredOn == false)
            {
                bulb.PoweredOn = true;
            }
        }
        static void Blinking()
        {
            port.Open();
            port.Write("2");
            port.Close();
        }
        static void process(object param)
        {
            try
            {
                var client = (TcpClient)param;
                var stream = client.GetStream();
                var sr = new StreamReader(stream);
                var sw = new StreamWriter(stream, Encoding.UTF8);

                string[] actionLine = sr.ReadLine()?.Split(new char[] { ' ' }, 3);
                int contentLength = 0;
                while (true)
                {
                    string line = sr.ReadLine();
                    string[] headLine = line?.Split(new char[] { ':' }, 2);
                    if (headLine[0] == "Content-Length")
                    {
                        contentLength = int.Parse(headLine[1].Trim());
                    }
                    Console.WriteLine(line);
                    if (string.IsNullOrWhiteSpace(line))
                        break;
                }
                string[] filePaths = Directory.GetFiles(path); 
                if (actionLine[0] == "POST")
                {
                    char[] postData = new char[contentLength];
                    sr.Read(postData, 0, contentLength);
                    Console.WriteLine(new string(postData));
                    string tmp = new string(postData);
                    string[] input = tmp.Split('=');
                    List<string> files = new List<string>();
                    foreach (var item in filePaths)
                    {
                        string[] items = item.Split('\\');
                        files.Add(items.Last());
                    }
                    foreach (var file in files)
                    {
                        if (input[1]==file)
                        {
                            foreach (var item in MimeTypes._mappings)
                            {
                                if (file.EndsWith(item.Key))
                                {    
                                    if (File.Exists(path +"\\"+ file.ToString()))
                                    {
                                        sw.WriteLine("HTTP/1.1 200 OK");
                                        sw.WriteLine($"Content-Type:{item.Value}; charset=utf-8");
                                        sw.WriteLine();
                                        byte[] filee = File.ReadAllBytes(path +"\\"+ file.ToString());
                                        sw.BaseStream.Write(filee, 0, filee.Length);
                                        sw.BaseStream.Flush();
                                    }
                                    else
                                    {
                                        sw.WriteLine("HTTP/1.1 404 NOT FOUND");
                                        sw.WriteLine();
                                    }
                                }
                            }
                        }
                    }
                    sw.Flush();
                    client.Close();
                }
                foreach (var  fgt in dic)
                {
                    if (actionLine[1] == fgt.Key)
                    {
                        SimpleDelegate tmp = fgt.Value;
                        tmp();
                    }
                }
                var test = actionLine[1].Split('/');
                if (test[1] == "jas")
                {
                    bulb.Brightness = Convert.ToInt32(test[2]);
                }
                if (actionLine[1]=="/Menu")
                {
                    Menu(sw, sr, filePaths);
                }
                foreach (var item in MimeTypes._mappings)
                {
                    if (actionLine[1].EndsWith(item.Key))
                    {
                        if (File.Exists(path + actionLine[1].ToString()))
                        {
                            sw.WriteLine("HTTP/1.1 200 OK");
                            sw.WriteLine($"Content-Type:{item.Value}; charset=utf-8");
                            sw.WriteLine();
                            byte[] file = File.ReadAllBytes(path + actionLine[1].ToString());
                            sw.BaseStream.Write(file, 0, file.Length);
                            sw.BaseStream.Flush();
                        }
                        else
                        {
                            sw.WriteLine("HTTP/1.1 404 NOT FOUND");
                            sw.WriteLine();
                        }

                    }
                }
                sw.Flush();
                client.Close();
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
            }

        }
        static void Menu(StreamWriter sw,StreamReader sr, string[] files)
        {
            sw.WriteLine("HTTP/1.1 200 OK");
            sw.WriteLine($"Content-Type:text/html; charset=utf-8");
            sw.WriteLine();
            sw.WriteLine("<html>");
            sw.WriteLine("<head>");
            sw.WriteLine("</head>");
            sw.WriteLine("<body>");
            sw.WriteLine("  <form action=\"/\" method=\"POST\">");
            sw.WriteLine("   <input type=\"text\" name=\"neco\" />");
            sw.WriteLine("   <input type=\"submit\" value=\"odeslat\" />");
            sw.WriteLine("  </form>");
            sw.WriteLine("<ul>");
            foreach (var item in files)
            {
                string[] tmp= item.Split('\\');
                sw.WriteLine($"<li>{item}</li><a href=\"{tmp.Last()}\">Otevřít</a>");
            }
            sw.WriteLine("</ul>");
            sw.WriteLine("</body>");
            sw.WriteLine("</html>");
        }
       
    }
}
