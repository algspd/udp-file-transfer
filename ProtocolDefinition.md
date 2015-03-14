# Introduction #

Here is the protocol design. This is only a draft.


# Packet types #

Fgetfrag packets are identified by its offsets.
There is no need to identify fgetinfo package because it is sent once and if no answer is received, communication doesn't start.
There is no need to identify fend because it closes connection and no answer is expected to reach the client.

## Client ##
  * fgetinfo:
    * type: (char) 1
    * file\_path: (char`[FILENAME_MAX]`) absolute file path
    * md5: (char`[33]`)

  * fgetfrag
    * type: (char) 2
    * file\_id: (int) assigned file id on finfo
    * offset:  (off\_t) wanted starting offset
    * md5:     (char`[33]`)

  * fend
    * type: (char) 3
    * file\_id: (int) assigned file id on finfo
    * md5:     (char`[33]`)


## Server ##
  * finfo
    * type: (char) 4
    * file\_exist: (int) 0 if file not found
    * file\_id:    (int) assigned file id (to avoid big paths on ffrag packets)
    * file\_size:  (off\_t) max offset, unknown if file\_exists=0
    * md5:        (char`[33]`)

  * ffrag
    * type: (char) 5
    * file\_id:  (int) assigned file id on finfo
    * offset:   (off\_t) starting offset
    * fragment: (char`[n]`) data from the file
    * md5:      (char`[33]`)


# Communication diagram #
```
 CLIENT      SERVER

fgetinfo    
             finfo
fgetfrag
             ffrag
fgetfrag
             ffrag
        ...
fend
```