CREATE TABLE `gps_data` (
  `id` int(11) NOT NULL,
  `deviceID` varchar(25) DEFAULT NULL,
  `longitude` float DEFAULT NULL,
  `latitude` float DEFAULT NULL,
  `altitude` float DEFAULT NULL,
  `speed` float DEFAULT NULL,
  `course` float DEFAULT NULL,
  `satellites` int(11) DEFAULT NULL,
  `time` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;


ALTER TABLE `gps_data`
  ADD PRIMARY KEY (`id`);

-- AUTO_INCREMENT für Tabelle `gps_data`
ALTER TABLE `gps_data`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=86;
