# Guide de Test - Snapshot Parsing

## Démarrage rapide

### 1. Compiler le projet

```powershell
# Depuis la racine du projet
cmake --build build --config Release --target r-type_server -j 4
cmake --build build --config Release --target r-type_client -j 4
```

### 2. Lancer le serveur

```powershell
cd build\server\Release
.\r-type_server.exe 50000
```

### 3. Lancer le client

```powershell
cd build\client\Release
.\r-type_client.exe 127.0.0.1 50000 Player1
```

## Résultats attendus

### Logs de connexion

```
[Network] TCP connection established
[Network] Sending CONNECT_REQ (44 bytes, username: Player1)
[Network] Connected. PlayerId=1, ServerUdpPort=50000
[Network] Successfully connected to server!
```

### Logs de snapshot (avec données parsées)

```
[UDP Snapshot] Tick=0 PayloadSize=12 bytes
  Raw payload (hex): 00 00 00 00 01 00 64 00 c8 00 2d 01
  Entities: 1
    [0] ID=0 Type=0x1 Pos=(100,200) Angle=301°
```

**Interprétation des données hex** :
- `00 00 00 00` : entity_id = 0 (little-endian uint32)
- `01` : entity_type = 1
- `00` : reserved byte
- `64 00` : pos_x = 100 (little-endian uint16)
- `c8 00` : pos_y = 200 (little-endian uint16)
- `2d 01` : angle = 301° (little-endian uint16)

### Logs de création d'entité

```
[Snapshot] Created entity #X for NetworkId=Y
```

## Vérifications

### ✅ Le parsing fonctionne si :
1. Les entités sont affichées avec leurs données complètes (ID, Type, Pos, Angle)
2. Des messages `[Snapshot] Created entity #...` apparaissent
3. Aucune erreur de parsing n'est affichée

### ❌ Problèmes possibles :

**"Entities: 0" alors que payload_size=12**
- Le format du serveur ne correspond pas au format attendu
- Solution : Vérifier que le client utilise le bon format (EntityState direct)

**Pas de message "[Snapshot] Created entity"**
- Les composants NetworkId ou Transform ne sont pas enregistrés
- Solution : Vérifier `InitRegistry()` pour l'enregistrement des composants

**Valeurs aberrantes (très grandes ou négatives)**
- Problème d'endianness ou de parsing
- Solution : Vérifier les logs "Raw payload" pour diagnostiquer

## Debug supplémentaire

Pour activer plus de logs, modifiez dans `ClientApplication.cpp` :

```cpp
// Afficher tous les snapshots (pas seulement tous les 10 ticks)
bool should_display = true;  // Au lieu de (display_count < 3) || ...
```

Pour voir le contenu du registre :

```cpp
// Après ApplySnapshotToRegistry
auto &network_ids = game_world.registry_.GetComponents<Component::NetworkId>();
auto &transforms = game_world.registry_.GetComponents<Component::Transform>();
std::cout << "[Debug] Registry has " << network_ids.size() 
          << " NetworkId components" << std::endl;
```

## Format des données serveur

Le serveur envoie actuellement via `PacketSender::SendSnapshot(EntityState)` :
- Header (12 bytes) avec opcode=0x20, payload_size=12, tick=0
- Payload (12 bytes) contenant un seul EntityState

Pour envoyer plusieurs entités, le serveur devra utiliser `WorldSnapshotPacket::Serialize()`.
