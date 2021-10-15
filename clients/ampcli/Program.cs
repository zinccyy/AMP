using System;
using Grpc.Core;
using Database;
using Player;
using Common;
using Sys;
using System.IO;

namespace GRPClient
{
    class Program
    {
        static async void TestDatabase(Channel channel)
        {
            var client = new Database.Database.DatabaseClient(channel);

            var call = client.GetAlbumCover(new AlbumRequest { Artist = "Mac Miller", Name = "Faces", Genre = "Hip-Hop" });
            var binaryWriter = new BinaryWriter(File.Open("cover.jpg", FileMode.OpenOrCreate));

            while (await call.ResponseStream.MoveNext())
            {
                binaryWriter.Write(call.ResponseStream.Current.Data.ToByteArray());
            }

            Console.WriteLine("Written to file cover.jpg");
        }

        static void TestPlayer(Channel channel)
        {
            var client = new Player.Player.PlayerClient(channel);
            // TODO : implement later
        }

        static void TestSystem(Channel channel)
        {
            var client = new Sys.Sys.SysClient(channel);
        }

        static void Main(string[] args)
        {
            Channel channel = new Channel("127.0.0.1:3000", ChannelCredentials.Insecure);

            // database
            TestDatabase(channel);

            // player
            TestPlayer(channel);

            // system
            TestSystem(channel);

            // channel.ShutdownAsync().Wait();
            Console.ReadKey();
        }
    }
}
