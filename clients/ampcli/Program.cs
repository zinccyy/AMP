using System;
using System.Net.Sockets;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace ampcli
{
    struct AddPath
    {
        [JsonPropertyName("path")]
        public string Path { get; set; }
    }
    struct CalcData
    {
        [JsonPropertyName("x")]
        public int X { get; set; }
        [JsonPropertyName("y")]
        public int Y { get; set; }
    }

    struct JSONRPCRequest
    {
        [JsonPropertyName("jsonrpc")]
        public string JSONRPC { get; set; }

        [JsonPropertyName("method")]
        public string Method { get; set; }

        [JsonPropertyName("params")]
        public Object Params { get; set; }

        [JsonPropertyName("id")]
        public int ID { get; set; }
    }
    class Program
    {
        static void Main(string[] args)
        {
            var calc = new CalcData
            {
                X = 10,
                Y = 20,
            };

            var req = new JSONRPCRequest
            {
                JSONRPC = "2.0",
                Method = "add_path",
                Params = new AddPath { Path = "E:\\Muzika" },
                ID = 1,
            };

            var jsonString = JsonSerializer.Serialize(req);
            Console.WriteLine(jsonString);

            try
            {
                TcpClient client = new TcpClient("127.0.0.1", 3000);

                // Translate the passed message into ASCII and store it as a Byte array.
                Byte[] data = System.Text.Encoding.ASCII.GetBytes(jsonString);

                // Get a client stream for reading and writing.
                //  Stream stream = client.GetStream();

                NetworkStream stream = client.GetStream();

                // Send the message to the connected TcpServer.
                stream.Write(data, 0, data.Length);

                Console.WriteLine("Sent: {0}", jsonString);

                // Receive the TcpServer.response.

                // Buffer to store the response bytes.
                data = new Byte[2048];

                // String to store the response ASCII representation.
                String responseData = String.Empty;

                // Read the first batch of the TcpServer response bytes.
                Int32 bytes = stream.Read(data, 0, data.Length);
                responseData = System.Text.Encoding.ASCII.GetString(data, 0, bytes);
                Console.WriteLine("Received: {0}", responseData);

                // Close everything.
                stream.Close();
                client.Close();
            }
            catch (ArgumentNullException e)
            {
                Console.WriteLine("ArgumentNullException: {0}", e);
            }
            catch (SocketException e)
            {
                Console.WriteLine("SocketException: {0}", e);
            }
        }
    }
}
