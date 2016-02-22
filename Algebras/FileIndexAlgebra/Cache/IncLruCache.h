/*
Dies ist eine einfache generische Implementierung eines LRU (Least recently used)-Caches.
Schluessel- und Wert-Typ werden per Template-Parameter festgelegt. Die Klasse bietet die
Moeglichkeit, Element in den Cache zu schreiben und heraus zulesen. Wenn ein Element aus dem
Cache entfernt wird, wird eine Callback-Methode aufgerufen.

Der komplette Code liegt hier im Header-File, weil templates nur umstaendlich auf
mehrere Code-Files zu verteilen sind.

Autor: David Ullrich

*/

#ifndef INCLRUCACHE_H
#define INCLRUCACHE_H

#include <functional>    // std::function
#include <list>          // std::list
#include <unordered_map> // std::unordered_map
#include <utility>       // std::pair

namespace fialgebra {
  namespace cache {
    namespace include {

      template<typename key_t, typename value_t>
      class IncLruCache
      {
      public:
        typedef typename std::pair<key_t, value_t> KeyValuePair;
        typedef typename std::list<KeyValuePair>   KeyValueList;
        typedef typename KeyValueList::iterator    KeyValueListIt;
        typedef typename std::unordered_map<key_t, KeyValueListIt> Map;
        typedef typename Map::iterator             MapIt;
        // Callback
        typedef std::function<void( key_t, value_t )> DelCallbackType;

        // ctor
        // maxSize     : Maximal Anzahl an Elementen, die der Cache
        //               aufnehmen kann.
        // delCallback : Delete callback, wird aufgerufen, wenn ein
        //               Element aus dem Cache entfernt wird.
        IncLruCache( size_t maxSize, DelCallbackType delCallback ) :
          _maxSize( maxSize ), _delCallback( delCallback ) { }
        ~IncLruCache() { }

        // Schreibt einen Wert in den Cache. Wenn ein Element aus dem
        // Cache weichen muss, wird der Delete callback aufgerufen.
        void Put( const key_t& key, const value_t& value ) {
          // Wenn das Element (der Key) bereits vorhanden ist,
          // wird es einfach entfernt.
          MapIt it = _map.find( key );
          if( it != _map.end() ) {
            // it     : MapIt
            // it->first  : key
            // it->second : value, hier KeyValueListIt

            // Callback
            _delCallback( it->second->first, it->second->second );

            _list.erase( it->second );
            _map.erase( it );
          } // if

          // Element am Anfang der Liste einfügen
          // Wichtig: Der Wert wird kopiert
          _list.push_front( KeyValuePair( key, value ) );
          // Der Iterator auf des (aktuell) erste Element
          // wird in die map geschrieben.
          _map[key] = _list.begin();

          // Wenn die maximale Größe überschritten ist,
          // wird das letzte (LRU) Element entfernt.
          if( _map.size() > _maxSize ) {
            KeyValueListIt last = _list.end();
            last--;

            // Callback
            _delCallback( last->first, last->second );

            // Element aus Map und aus Liste entfernen
            _map.erase( last->first );
            _list.pop_back();
          } // if
        }
        // Liest einen Wert aus dem Cache. Wirft eine Exception,
        // wenn der Schlüssel nicht im Cache existiert.
        const value_t& Get( const key_t& key ) {
          // find() liefert den Iterator auf des gesuchte Element,
          // nicht direkt das Element.
          MapIt it = _map.find( key );
          if( it == _map.end() )
            throw std::range_error( "Schluessen nicht in Cache gefunden" );

          // it->first  : key
          // it->second : value, hier KeyValueListIt

          // Das gesuchte Element wird wieder an den Anfang
          // der Liste geschoben.
          _list.splice( _list.begin(), _list, it->second );
          return it->second->second;
        }

        // Leer den Cache komplett. Für jedes Element wird der
        // Delete callback aufgerufen.
        void Clear() {
          // Fuer jedes Element wird der Delete callback
          // aufgerufen. Anschliessend wird die map geleert.
          for( MapIt it = _map.begin(); it != _map.end(); it++ )
            _delCallback( it->second->first, it->second->second );

          _map.clear();
          _list.clear();
        }

        // Prüft, ob der Schlüssel im Cache enthalten ist.
        bool Contains( const key_t& key ) const {
          // find() liefet einen iterator auf das gesuchte Element.
          return _map.find( key ) != _map.end();
        }

        // Maximal Anzahl an Elementen, die der Cache aufnehmen kann.
        size_t MaxSize() const {
          return _maxSize;
        }
        // Aktuelle Anzahl an Elementen im Cache.
        size_t Size() const {
          return _map.size();
        }

      private:
        // Maximale Anzahl Elemente
        size_t _maxSize;

        KeyValueList _list;
        Map _map;
        // Delete callback
        DelCallbackType _delCallback;
      };

    } // namespace include
  } // namespace cache
} // namespace fialgebra

#endif // INCLRUCACHE_H















