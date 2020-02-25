- Working with WebSockets from secondo:

     open database berlintest;

    Examples from https://www.blockchain.com/api/api_websocket
        query readfromwebsocket('wss://ws.blockchain.info/inv', '{"op":"unconfirmed_sub"}', 'Operation : string /op') finishStream[8080] head[10] consume

        query readfromwebsocket('wss://ws.blockchain.info/inv', '{"op":"unconfirmed_sub"}',
            'Operation : string /op,
            Size : int /x/size,
            Addr : string /x/inputs/0/prev_out/addr')
            finishStream[8080] consoleConsumer head[10] count;

    Example from https://www.safe.com/
        query readfromwebsocket('https://demos-safe-software.fmecloud.com:7078/websocket',
         '{"ws_op":"open","ws_stream_id":"sd_ship"}',
            'Name : string /vessel_name,
             Latitude : real /latitude,
             Longitude : real /longitude')
            finishStream[8080] consoleConsumer count;

    Example from https://www.smartcolumbusos.com
        query readfromwebsocket('wss://streams.smartcolumbusos.com/socket/websocket',
             '{"topic":"streaming:central_ohio_transit_authority__cota_stream","event":"phx_join","payload":{},"ref":1}',
             'Label : string /payload/vehicle/vehicle/label,
              Id : string /payload/vehicle/vehicle/id,
              Latitude : real /payload/vehicle/position/latitude,
              Longitude : real /payload/vehicle/position/longitude')
             finishStream[8080] consoleConsumer head[100] consume;

