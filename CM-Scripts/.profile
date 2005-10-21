# local profile for MSYS

# assert that a default user name
# and home directory exists
if [ "$LOGNAME" == "" ]; then
  echo -e "\$LOGNAME is empty! Set to \"nodbody\"."
  LOGNAME="nobody"
fi

export USER=$LOGNAME
export HOME="/home/$LOGNAME"

if [ ! -d "$HOME" ]; then
  if ! mkdir -p "$HOME"; then
    echo -e "ERROR: Could not create directory \$HOME=\"$HOME\"."
    echo -e "       You will have no home directory."
  fi
fi

# simulate linux like bash behaviour
if [ -f $HOME/.bashrc ]; then
   source $HOME/.bashrc
fi
cd "$HOME"
