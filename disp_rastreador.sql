-- phpMyAdmin SQL Dump
-- version 5.2.1
-- https://www.phpmyadmin.net/
--
-- Host: localhost
-- Tempo de geração: 24/11/2023 às 02:35
-- Versão do servidor: 10.4.28-MariaDB
-- Versão do PHP: 8.2.4

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Banco de dados: `disp_rastreador`
--

-- --------------------------------------------------------

--
-- Estrutura para tabela `data`
--

CREATE TABLE `data` (
  `id` int(11) NOT NULL,
  `deviceID` int(11) NOT NULL,
  `dateTime` varchar(25) NOT NULL,
  `longitude` varchar(20) NOT NULL,
  `latitude` varchar(20) NOT NULL,
  `acx` varchar(20) NOT NULL,
  `acy` varchar(20) NOT NULL,
  `acz` varchar(20) NOT NULL,
  `gyx` varchar(20) NOT NULL,
  `gyy` varchar(20) NOT NULL,
  `gyz` varchar(20) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- Despejando dados para a tabela `data`
--

INSERT INTO `data` (`id`, `deviceID`, `dateTime`, `longitude`, `latitude`, `acx`, `acy`, `acz`, `gyx`, `gyy`, `gyz`) VALUES
(18, 12, '2022-04-22 10:34:23.55', '100.0', '150.0', '25.4', '56.8', '32.9', '77.2', '89.1', '27.7'),
(24, 12, '2022-04-22 10:34:23.55', '100.0', '150.0', '25.4', '56.8', '32.9', '77.2', '89.1', '27.7'),
(25, 12, '2022-04-22 10:34:23.55', '100.0', '150.0', '25.4', '56.8', '32.9', '77.2', '89.1', '27.7'),
(26, 12, '2022-04-22 10:34:23.55', '100.0', '150.0', '25.4', '56.8', '32.9', '77.2', '89.1', '27.7'),
(27, 12, '2022-04-22 10:34:23.55', '100.0', '150.0', '25.4', '56.8', '32.9', '77.2', '89.1', '27.7'),
(28, 12, '2022-04-22 10:34:23.55', '100.0', '150.0', '25.4', '56.8', '32.9', '77.2', '89.1', '27.7');

-- --------------------------------------------------------

--
-- Estrutura para tabela `device`
--

CREATE TABLE `device` (
  `id` int(11) NOT NULL,
  `MAC` varchar(17) NOT NULL,
  `userID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- Despejando dados para a tabela `device`
--

INSERT INTO `device` (`id`, `MAC`, `userID`) VALUES
(10, '00:1B:44:11:3A:B7', 18),
(12, '11:1B:44:11:3A:B7', 18);

-- --------------------------------------------------------

--
-- Estrutura para tabela `user`
--

CREATE TABLE `user` (
  `id` int(11) NOT NULL,
  `name` varchar(30) NOT NULL,
  `email` varchar(50) NOT NULL,
  `password` varchar(30) NOT NULL,
  `vfCoordinates` varchar(125) NOT NULL,
  `token` varchar(32) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- Despejando dados para a tabela `user`
--

INSERT INTO `user` (`id`, `name`, `email`, `password`, `vfCoordinates`, `token`) VALUES
(18, 'Guilherme Kraemer', 'guilherme.kraemer@ecomp.ufsm.br', '12345678', '-30.11642, -51.33490, -30.10636, -51.33979, 30.10893, -51.34276', '840526687'),
(22, 'Guilherme Kraemer', 'gkraemer@ecomp.ufsm.br', '1asdsd5678', '', '413199984');

--
-- Índices para tabelas despejadas
--

--
-- Índices de tabela `data`
--
ALTER TABLE `data`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `id` (`id`),
  ADD KEY `data_ibfk_1` (`deviceID`);

--
-- Índices de tabela `device`
--
ALTER TABLE `device`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `id` (`id`),
  ADD UNIQUE KEY `MAC` (`MAC`),
  ADD KEY `device_ibfk_1` (`userID`);

--
-- Índices de tabela `user`
--
ALTER TABLE `user`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `id` (`id`),
  ADD UNIQUE KEY `email` (`email`),
  ADD KEY `password` (`password`) USING BTREE;

--
-- AUTO_INCREMENT para tabelas despejadas
--

--
-- AUTO_INCREMENT de tabela `data`
--
ALTER TABLE `data`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=31;

--
-- AUTO_INCREMENT de tabela `device`
--
ALTER TABLE `device`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=15;

--
-- AUTO_INCREMENT de tabela `user`
--
ALTER TABLE `user`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=23;

--
-- Restrições para tabelas despejadas
--

--
-- Restrições para tabelas `data`
--
ALTER TABLE `data`
  ADD CONSTRAINT `data_ibfk_1` FOREIGN KEY (`deviceID`) REFERENCES `device` (`id`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Restrições para tabelas `device`
--
ALTER TABLE `device`
  ADD CONSTRAINT `device_ibfk_1` FOREIGN KEY (`userID`) REFERENCES `user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
