using System;
using Grpc.Core;
using Player;
using Common;
using Database;
using Sys;

namespace GRPClient
{
    class Program
    {
        static void TestDatabase(Channel channel)
        {
            var client = new Database.Database.DatabaseClient(channel);

            var albums = client.GetAlbums(new Common.Empty());
            var artists = client.GetArtists(new Common.Empty());
            var genres = client.GetGenres(new Common.Empty());

            Console.WriteLine("Albums: ");
            foreach(var album in albums.Albums)
            {
                Console.WriteLine("\t" + album.Artist + ": " + album.Name);
            }

            Console.WriteLine("Artists: ");
            foreach (var artist in artists.Artists)
            {
                Console.WriteLine("\t" + artist.Name + ": " + artist.CoverPath);
            }

            Console.WriteLine("Genres: ");
            foreach(var genre in genres.Genres)
            {
                Console.WriteLine("\t" + genre.Name);
            }
        }

        static void TestPlayer(Channel channel)
        {
            var client = new Player.Player.PlayerClient(channel);

            // TODO : implement later
        }

        static void Main(string[] args)
        {
            Channel channel = new Channel("127.0.0.1:3000", ChannelCredentials.Insecure);

            // database
            TestDatabase(channel);


            channel.ShutdownAsync().Wait();
            Console.ReadKey();
        }
    }
}
