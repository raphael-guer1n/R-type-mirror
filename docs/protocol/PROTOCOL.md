# 📡 R-Type Network Protocol

## 1. Introduction

Ce document définit le protocole réseau utilisé par **R-Type**.  

- **Transport** : UDP (User Datagram Protocol)  
- **Format** : binaire (pas de JSON/texte)  
- **Autorité** : le serveur est la source de vérité  
- **Rôle du client** : envoyer ses inputs, afficher les snapshots reçus  

---

## 2. Header commun

Tous les paquets commencent par un header de **8 octets** :

```
#pragma pack(push, 1)
struct PacketHeader {
    uint16_t type;   -> identifiant du paquet (enum)
    uint16_t size;   -> taille du payload en octets
    uint32_t seq;    -> numéro de séquence
};
#pragma pack(pop)

```

type : identifiant du paquet (enum).
size : taille du payload en octets.
seq : numéro de séquence, incrémenté par l’émetteur.

## 3. Types de paquets

### 3.1 CONNECT_REQ (Client → Serveur)
Type = 1
Payload :

```

struct ConnectReq {
    uint32_t clientId;  -> identifiant temporaire choisi par le client
};
```

But : demander une connexion au serveur.

### 3.2 CONNECT_ACK (Serveur → Client)
Type = 2
Payload :
```

struct ConnectAck {
    uint32_t serverId;   -> identifiant attribué par le serveur
    uint32_t tickRate;   -> fréquence du serveur (Hz, ex: 60)
};
```

But : confirmer la connexion et transmettre paramètres initiaux.

### 3.3 INPUT (Client → Serveur)
Type = 3
Payload :
```

struct InputPacket {
    uint32_t clientId;  -> id du joueur
    uint32_t tick;      -> tick local au moment de l’input
    uint8_t  keys;      -> bitmap des touches
};
```

Mapping des touches (exemple) :
bit 0 = UP
bit 1 = DOWN
bit 2 = LEFT
bit 3 = RIGHT
bit 4 = SHOOT

### 3.4 SNAPSHOT (Serveur → Client)
Type = 4
Payload :
```

struct EntityState {
    uint32_t entityId;
    float    x, y;
    float    vx, vy;
    uint8_t  type;    -> joueur, ennemi, projectile…
    uint8_t  hp;
};

struct Snapshot {
    uint32_t tick;          -> tick du serveur
    uint16_t entityCount;   -> nombre d’entités
    EntityState entities[]; -> tableau variable
};
```

But : envoyer l’état du monde pour ce tick.

### 3.5 EVENT (Serveur → Client)
Type = 5
Payload générique :
```

struct EventPacket {
    uint32_t tick;
    uint16_t eventType;   -> ex: 1=PlayerDeath, 2=Spawn, 3=PowerUp
    uint32_t entityId;
};
```

But : signaler un événement ponctuel.

### 3.6 PING / PONG
Type = 6 / 7
Payload :
```

struct PingPacket {
    uint64_t timestamp;
};
```

But : mesurer la latence et maintenir la connexion active.

## 4. Exemple binaire
Exemple d’un paquet INPUT :
- Header :
    - type = 0x0003 (INPUT)
    - size = 0x0009 (9 octets payload)
    - seq = 0x00000005

- Payload :
    - clientId = 0x00000001
    - tick = 0x0000003C (60 décimal)
    - keys = 0b00010000 (tir)
```
Représentation hex :
03 00 09 00 05 00 00 00  01 00 00 00  3C 00 00 00  10
```

## 5. Fiabilité

- Le protocole repose sur UDP (pas de garantie).
- Les paquets critiques (connexion) doivent être réémis si pas de réponse (timeout).
- Les autres paquets (inputs, snapshots) sont envoyés très fréquemment → perte tolérable.

## 6. Règles générales

- Le serveur est autoritaire : seule sa vision fait foi.
- Le client doit :
    - envoyer ses inputs régulièrement (~60 Hz),
    - afficher les snapshots reçus,
    - gérer la fluidité via interpolation/prédiction locale.
- Toute donnée inconnue doit être ignorée (robustesse).