CREATE TABLE Artist
(
  artist_id INT NOT NULL,
  artist_name VARCHAR(256) NOT NULL,
  artist_image VARCHAR(2048),
  PRIMARY KEY (artist_id)
);

CREATE TABLE Genre
(
  genre_id INT NOT NULL,
  genre_name VARCHAR(256) NOT NULL,
  PRIMARY KEY (genre_id)
);

CREATE TABLE Album
(
  album_id INT NOT NULL,
  album_name VARCHAR(256) NOT NULL,
  album_cover VARCHAR(2048),
  album_year DATE NOT NULL,
  album_folder_path VARCHAR(2048) NOT NULL,
  artist_id INT NOT NULL,
  genre_id INT NOT NULL,
  PRIMARY KEY (album_id),
  FOREIGN KEY (artist_id) REFERENCES Artist(artist_id),
  FOREIGN KEY (genre_id) REFERENCES Genre(genre_id)
);

CREATE TABLE Track
(
  track_id INT NOT NULL,
  track_title VARCHAR(256) NOT NULL,
  track_length TIME,
  track_number INT NOT NULL,
  track_comment VARCHAR(256) NOT NULL,
  track_bit_rate INT,
  track_sample_rate INT,
  track_channels INT,
  track_file_path VARCHAR(2048) NOT NULL,
  album_id INT NOT NULL,
  PRIMARY KEY (track_id),
  FOREIGN KEY (album_id) REFERENCES Album(album_id)
);

CREATE TABLE Features
(
  track_id INT NOT NULL,
  artist_id INT NOT NULL,
  PRIMARY KEY (track_id, artist_id),
  FOREIGN KEY (track_id) REFERENCES Track(track_id),
  FOREIGN KEY (artist_id) REFERENCES Artist(artist_id)
);
