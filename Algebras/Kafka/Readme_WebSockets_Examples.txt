Working with WebSockets from secondo

 For all tests, first open some database, e.g.:
     open database berlintest;

  -- Test with mock data
  Mock data is constant object:
      {
         "application": "hiking",
         "reputons": [
         {
             "rater": "HikingAsylum",
             "assertion": "advanced",
             "rated": "Marilyn C",
             "rating": 0.90
           }
         ]
      }

    Read 10 messages:
     query readfromwebsocket('mock://data', 'hello', 'Name string /reputons/0/rated')
        finishStream[8080] head[10] consume;

    Read continuously:
     query readfromwebsocket('mock://data', 'hello', 'Name string /reputons/0/rated')
        finishStream[8080] consoleConsumer count;
    From other Secondo terminal:
     query signalFinish("localhost", 8080);

  -- Blockchain Examples from https://www.blockchain.com/api/api_websocket

    Read first 10 messages and extract "/op" data
        query readfromwebsocket('wss://ws.blockchain.info/inv', '{"op":"unconfirmed_sub"}',
            'Operation : string /op') finishStream[8080] head[10] consume

    Read first 10 messages and extract data from 3 places
        query readfromwebsocket('wss://ws.blockchain.info/inv', '{"op":"unconfirmed_sub"}',
            'Operation : string /op,
            Size : int /x/size,
            Addr : string /x/inputs/0/prev_out/addr')
            finishStream[8080] consoleConsumer head[10] count;

  -- Blockchain Examples from wss://ws-feed.pro.coinbase.com

    Read first 10 messages and extract data from 3 places
        query readfromwebsocket('wss://ws-feed.pro.coinbase.com',
                '{"type":"subscribe","product_ids":["ETH-USD","ETH-EUR"],"channels":["level2"]}',
                'Product : string /product_id,
                Operation : string /changes/0/0,
                Amount : string /changes/0/1')
            finishStream[8080] head[10] consume

  -- Example from https://www.safe.com/

    Possible requests:
    {"ws_op":"open","ws_stream_id":"sd_ship"}
    {"ws_op":"open","ws_stream_id":"sd_bus"}
    {"ws_op":"open","ws_stream_id":"sd_plane"}

    Read first 10 ships data with name and geo position
        query readfromwebsocket('https://demos-safe-software.fmecloud.com:7078/websocket',
         '{"ws_op":"open","ws_stream_id":"sd_ship"}',
            'Name : string /vessel_name,
             Latitude : real /latitude,
             Longitude : real /longitude')
            finishStream[8080] head[10] consume;


    Create a ship relation and import data into it:
        let ships = [ const rel(tuple([Name : string, Latitude : real, Longitude : real])) value () ];

        query readfromwebsocket('https://demos-safe-software.fmecloud.com:7078/websocket',
         '{"ws_op":"open","ws_stream_id":"sd_ship"}',
            'Name : string /vessel_name,
             Latitude : real /latitude,
             Longitude : real /longitude')
            finishStream[8080] consoleConsumer ships insert count;
    From other Secondo terminal:
        query signalFinish("localhost", 8080);

  -- Bus realtime positions. Example from https://www.smartcolumbusos.com

    Read first 100 messages:
        query readfromwebsocket('wss://streams.smartcolumbusos.com/socket/websocket',
             '{"topic":"streaming:central_ohio_transit_authority__cota_stream","event":"phx_join","payload":{},"ref":1}',
             'Label : string /payload/vehicle/vehicle/label,
              Id : string /payload/vehicle/vehicle/id,
              Latitude : real /payload/vehicle/position/latitude,
              Longitude : real /payload/vehicle/position/longitude')
             finishStream[8080] consoleConsumer head[100] consume;

    Example with relation creation

        let columbus_buses = [ const rel(tuple([Label : string, Id : string, Latitude : real, Longitude : real])) value () ];
        query readfromwebsocket('wss://streams.smartcolumbusos.com/socket/websocket',
             '{"topic":"streaming:central_ohio_transit_authority__cota_stream","event":"phx_join","payload":{},"ref":1}',
             'Label : string /payload/vehicle/vehicle/label,
              Id : string /payload/vehicle/vehicle/id,
              Latitude : real /payload/vehicle/position/latitude,
              Longitude : real /payload/vehicle/position/longitude')
             finishStream[8080] consoleConsumer head[100] columbus_buses insert consume;

    From other Secondo terminal:
        query signalFinish("localhost", 8080);

    After that the data can be visualized:
        query columbus_buses feed head[100] projectextend[Label; Pos: makepoint(.Longitude, .Latitude)] consume


