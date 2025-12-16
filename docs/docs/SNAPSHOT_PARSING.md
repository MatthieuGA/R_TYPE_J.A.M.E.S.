# Snapshot Parsing - Client Side

## Vue d'ensemble

Le client reçoit des snapshots UDP du serveur contenant l'état des entités du monde. Ce document explique comment ces données sont parsées et appliquées au registre ECS.

## Structure du Snapshot

### Format du paquet

Un snapshot UDP (opcode `0x20`) contient :

```
Header UDP (12 bytes) :
- opcode (1 byte) = 0x20
- payload_size (2 bytes)
- tick (4 bytes)
- reserved (5 bytes)

Payload (format actuel) :
- Single EntityState (12 bytes) directement
```

**Note**: Le serveur envoie actuellement un seul `EntityState` par paquet au lieu d'un `WorldSnapshotPacket` complet. Le format complet avec `entity_count` est supporté pour compatibilité future.

### Format WorldSnapshotPacket (futur/optionnel)

```
Payload WorldSnapshotPacket :
- entity_count (2 bytes)
- reserved (2 bytes)
- entities[] (12 bytes chacun)
```

### Structure EntityState (12 bytes)

Chaque entité dans le snapshot est encodée sur 12 bytes :

| Offset | Taille | Type   | Description                          |
|--------|--------|--------|--------------------------------------|
| 0-3    | 4      | uint32 | entity_id                            |
| 4      | 1      | uint8  | entity_type (sprite/prefab ID)       |
| 5      | 1      | uint8  | reserved (padding)                   |
| 6-7    | 2      | uint16 | pos_x (normalisé 0..65535)           |
| 8-9    | 2      | uint16 | pos_y (normalisé 0..38864)           |
| 10-11  | 2      | uint16 | angle (degrés 0..360)                |

## Implémentation Client

### 1. Parsing des données

La fonction `ParseSnapshotData()` extrait les entités du payload binaire :

```cpp
std::vector<ParsedEntity> ParseSnapshotData(
    const client::SnapshotPacket &snapshot);
```

**Exemple d'utilisation :**
```cpp
auto snapshot = server_connection_->PollSnapshot();
if (snapshot.has_value()) {
    auto entities = ParseSnapshotData(snapshot.value());
    
    for (const auto &entity : entities) {
        std::cout << "Entity ID: " << entity.entity_id << std::endl;
        std::cout << "Position: (" << entity.pos_x << ", " 
                  << entity.pos_y << ")" << std::endl;
        std::cout << "Angle: " << entity.angle << "°" << std::endl;
    }
}
```

### 2. Application au registre ECS

La fonction `ApplySnapshotToRegistry()` synchronise les entités du registre avec les données du serveur :

```cpp
void ApplySnapshotToRegistry(GameWorld &game_world,
                              const client::SnapshotPacket &snapshot);
```

**Comportement :**
- Crée de nouvelles entités si elles n'existent pas (avec composants `NetworkId` et `Transform`)
- Met à jour les composants `Transform` existants (position et rotation)

**Exemple d'utilisation :**
```cpp
auto snapshot = server_connection_->PollSnapshot();
if (snapshot.has_value()) {
    ApplySnapshotToRegistry(game_world, snapshot.value());
}
```

### 3. Affichage de debug

La fonction `DisplaySnapshotData()` affiche les informations pour le debug :

```cpp
void DisplaySnapshotData(const client::SnapshotPacket &snapshot);
```

**Output :**
```
[UDP Snapshot] Tick=120 PayloadSize=16 bytes
  Entities: 1
    [0] ID=42 Type=0x1 Pos=(100,200) Angle=45°
```

## Intégration dans la boucle de jeu

Dans `ClientApplication::RunGameLoop()` :

```cpp
// Poll and apply UDP snapshots
if (game_world.server_connection_) {
    auto snapshot = game_world.server_connection_->PollSnapshot();
    if (snapshot.has_value()) {
        // Appliquer au registre
        ApplySnapshotToRegistry(game_world, snapshot.value());
        
        // Affichage debug (optionnel)
        DisplaySnapshotData(snapshot.value());
    }
}
```

## Structure ParsedEntity

Les données parsées sont stockées dans une structure intermédiaire :

```cpp
struct ParsedEntity {
    uint32_t entity_id;     // ID unique de l'entité
    uint8_t entity_type;    // Type (sprite/prefab)
    uint16_t pos_x;         // Position X (normalisée)
    uint16_t pos_y;         // Position Y (normalisée)
    uint16_t angle;         // Angle de rotation (0..360°)
};
```

## Composants ECS synchronisés

### NetworkId Component
```cpp
struct NetworkId {
    int id;  // ID correspondant à entity_id du serveur
};
```

### Transform Component
```cpp
struct Transform {
    float x;                  // Position X (mise à jour depuis pos_x)
    float y;                  // Position Y (mise à jour depuis pos_y)
    float rotationDegrees;    // Rotation (mise à jour depuis angle)
    sf::Vector2f scale;       // Échelle
    // ... autres champs
};
```

## Gestion des entités

1. **Nouvelle entité** : Si aucune entité avec le `NetworkId` correspondant n'existe :
   - Création avec `registry_.SpawnEntity()`
   - Ajout de `NetworkId` et `Transform`
   - Log : `[Snapshot] Created entity #X for NetworkId=Y`

2. **Entité existante** : Si une entité avec le `NetworkId` existe :
   - Mise à jour du composant `Transform`
   - Les positions et rotation sont synchronisées

## Notes importantes

- **Thread Safety** : Les snapshots sont lus via une queue lock-free (`boost::lockfree::spsc_queue`)
- **Fréquence d'affichage** : L'affichage debug est limité (tous les 10 ticks) pour éviter le spam
- **Normalisation** : Les coordonnées reçues sont des uint16 et doivent être converties en float
- **Tick** : Le numéro de tick permet de suivre l'ordre des snapshots et détecter les pertes de paquets

## Référence RFC

Voir `rfcs/PROTOCOL.md` section 6.2 pour la spécification complète du protocole WorldSnapshot.
