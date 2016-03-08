#pragma once

class GameException
{
};

class ArrowAlreadyPreparedException : public GameException
{
};

class ArrowDoubleBackException : public GameException
{
};

class ArrowPathLengthException : public GameException
{
};

class NoSuchRoomException : public GameException
{
};

class OutOfArrowsException : public GameException
{
};

class PlayerDeadException : public GameException
{
};

class RoomsNotConnectedException : public GameException
{
};
