# Charging Show Asset System

## Description

Le système `ChargingShowAssetPlayerSystem` gère l'affichage visuel de l'asset de charge pour les joueurs. Il contrôle dynamiquement l'opacité des entités enfants (typiquement l'effet de charge) en fonction du temps de charge accumulé par le joueur.

## Fichier Source

- **Path**: `client/engine/systems/SystemsFunctions/ChargingShowAssetPlayerSystem.cpp`
- **Header**: `client/engine/systems/InitRegistrySystems.hpp`

## Composants Utilisés

| Composant | Type | Description |
|-----------|------|-------------|
| `PlayerTag` | Read/Write | Contient le temps de charge actuel du joueur |
| `Drawable` | Write | Modifié pour ajuster l'opacité de l'asset de charge |
| `AnimatedSprite` | Write | Réinitialise la frame d'animation lors du début de charge |
| `Transform` | Read | Utilisé pour accéder à la liste des enfants de l'entité joueur |

## Comportement

### Logique Principale

1. **Itération sur les Joueurs**: Pour chaque entité ayant un `PlayerTag` et un `Transform`:
   - Vérifie si l'entité a des enfants dans sa liste `transform.children`
   - Récupère le premier enfant (asset de charge)

2. **Mise à Jour de l'Opacité**:
   - Si `player_tag.charge_time > player_tag.charge_time_min`:
     - L'opacité du drawable enfant est mise à **1.0** (visible)
   - Sinon:
     - L'opacité du drawable enfant est mise à **0.0** (invisible)

3. **Réinitialisation de l'Animation**:
   - Lorsque l'opacité passe de 0 à visible, la frame d'animation est réinitialisée à 0

### Fonction Auxiliaire

#### `SetOpacityChildren`

```cpp
void SetOpacityChildren(
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites,
    Com::PlayerTag &player_tag,
    Eng::sparse_array<Com::Transform> &transforms,
    size_t child_id
)
```

**Rôle**: Configure l'opacité et l'état d'animation d'une entité enfant spécifique.

**Paramètres**:

- `drawables`: Tableau sparse des composants Drawable
- `animated_sprites`: Tableau sparse des composants AnimatedSprite
- `player_tag`: Tag du joueur parent contenant le temps de charge
- `transforms`: Tableau sparse des composants Transform (non utilisé actuellement)
- `child_id`: ID de l'entité enfant à modifier

**Logique**:

1. Si l'opacité actuelle est 0, réinitialise `currentFrame` à 0
2. Met à jour l'opacité selon la condition `charge_time > charge_time_min`

## Architecture Parent-Enfant

Ce système utilise la relation hiérarchique entre entités:

- **Entité Parent**: Le joueur (possède `PlayerTag`, `Transform`)
- **Entité Enfant**: L'asset de charge (possède `Drawable`, `AnimatedSprite`, `Transform`)

La liste `Transform.children` contient les IDs des entités enfants, permettant un accès direct sans parcourir tous les transforms.

## Exemple d'Utilisation

### Configuration Initiale

Lors de la création du joueur dans `initGameLevel.cpp`:

```cpp
// Créer le joueur
auto player_entity = reg.SpawnEntity();
reg.AddComponent<Component::Transform>(player_entity, {...});
reg.AddComponent<Component::PlayerTag>(player_entity, {...});

// Créer l'asset de charge (enfant)
auto player_charging_entity = reg.SpawnEntity();
reg.AddComponent<Component::Transform>(player_charging_entity,
    Component::Transform(..., player_entity.GetId())); // Parent ID
reg.AddComponent<Component::Drawable>(player_charging_entity, {...});
reg.AddComponent<Component::AnimatedSprite>(player_charging_entity, {...});

// Ajouter l'enfant à la liste du parent
reg.GetComponent<Component::Transform>(player_entity)
    .children.push_back(player_charging_entity.GetId());
```

### Fonctionnement en Jeu

1. Le joueur maintient la touche de tir (`Space`)
2. Le `ShootPlayerSystem` incrémente `player_tag.charge_time`
3. Lorsque `charge_time` dépasse `charge_time_min` (0.5s par défaut):
   - `ChargingShowAssetPlayerSystem` rend l'asset visible (`opacity = 1.0`)
   - L'animation de charge devient visible à l'écran
4. Lorsque le joueur relâche le tir:
   - `charge_time` est réinitialisé à 0
   - L'asset redevient invisible (`opacity = 0.0`)

## Performance

**Complexité**: O(n × m) où:

- n = nombre d'entités avec `PlayerTag` et `Transform`
- m = nombre moyen d'enfants par joueur (généralement 1)

**Optimisation**: Utilise `transform.children` pour accès direct plutôt que de parcourir tous les transforms du registre.

## Relations avec Autres Systèmes

| Système | Relation |
|---------|----------|
| `ShootPlayerSystem` | Met à jour `charge_time` qui est lu par ce système |
| `AnimationSystem` | Anime l'asset de charge lorsqu'il est visible |
| `DrawableSystem` | Rend l'asset avec l'opacité définie par ce système |

## Notes Techniques

- Le système suppose que le premier enfant (`children[0]`) est l'asset de charge
- Si l'entité n'a pas d'enfants, elle est ignorée sans erreur
- L'opacité est binaire (0.0 ou 1.0) mais pourrait être rendue progressive
- Le composant `Transform` des enfants n'est pas actuellement utilisé dans `SetOpacityChildren`

## Améliorations Potentielles

1. **Opacité Progressive**: Interpoler l'opacité en fonction du temps de charge
2. **Support Multi-Enfants**: Gérer plusieurs assets de charge avec différents seuils
3. **Effets Visuels**: Ajouter des effets de particules ou de lueur pendant la charge
4. **Son**: Intégrer un feedback audio lors de l'activation de la charge
