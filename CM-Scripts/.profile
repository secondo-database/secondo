# local profile for MSYS

if [ "x$LOGNAME" == "x" ]; then
  LOGNAME="unknown"
fi
export USER=$LOGNAME

# simulate linux like bash behaviour
if [ -f $HOME/.bashrc ]; then
   source $HOME/.bashrc
fi
cd "$HOME"
