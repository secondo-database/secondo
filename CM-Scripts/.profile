# local profile for MSYS

# assert that a default user name
# and home directory exists
if [ "x$LOGNAME" == "x" ]; then
  LOGNAME="unknown"
fi

export USER=$LOGNAME
export HOME="/home/$LOGNAME"

if [ ! -d "$HOME" ]; then
  mkdir -p "$HOME"
fi

# simulate linux like bash behaviour
if [ -f $HOME/.bashrc ]; then
   source $HOME/.bashrc
fi
cd "$HOME"
