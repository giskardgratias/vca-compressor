# 🎛️ VCA Compressor — Guide de mise en ligne GitHub

Suis ces étapes dans l'ordre. **Aucun logiciel à installer sur ton PC.**

---

## Étape 1 — Créer un compte GitHub (si pas déjà fait)

1. Va sur **https://github.com**
2. Clique **Sign up** → crée un compte gratuit
3. Vérifie ton email

---

## Étape 2 — Créer un nouveau dépôt (repository)

1. Une fois connecté, clique le **+** en haut à droite → **New repository**
2. Remplis :
   - **Repository name** : `vca-compressor`
   - **Visibility** : `Public` ✅ (nécessaire pour les Actions gratuites)
   - Ne coche rien d'autre
3. Clique **Create repository**

---

## Étape 3 — Uploader les fichiers sources

Sur la page de ton dépôt vide, clique **uploading an existing file**.

Décompresse le ZIP que tu as reçu et uploade **tous les fichiers** en conservant
l'arborescence :

```
.github/
    workflows/
        build.yml          ← IMPORTANT
source/
    entry.cpp
    VCACompressorDSP.h
    VCACompressorDSP.cpp
    VCACompressorProcessor.h
    VCACompressorProcessor.cpp
    VCACompressorController.h
    VCACompressorController.cpp
    VCACompressorIDs.h
    version.h
CMakeLists.txt
.gitignore
README.md
```

> 💡 **Astuce** : Sélectionne tous les fichiers d'un coup avec Ctrl+A et
> fais-les glisser dans la zone d'upload GitHub.

Descends en bas, écris un message comme `"Initial commit"` et clique
**Commit changes**.

---

## Étape 4 — Vérifier que le build démarre automatiquement

1. Clique l'onglet **Actions** en haut de ton dépôt
2. Tu dois voir un workflow **"Build VCA Compressor VST3 + Installer"** en cours
   (icône orange ⏳)
3. Attends ~10–15 minutes que ça se termine
4. Si l'icône passe au ✅ vert → **succès !**
5. Si ❌ rouge → clique dessus pour voir l'erreur (et envoie-moi le message)

---

## Étape 5 — Télécharger ton .exe installeur

1. Clique sur le run ✅ dans l'onglet Actions
2. Descends jusqu'à la section **Artifacts**
3. Clique **VCACompressor-Windows-x64** → téléchargement automatique d'un ZIP
4. Décompresse → tu as ton **`VCACompressor_v1.0.0_Setup.exe`** 🎉

---

## Étape 6 (optionnel) — Créer une Release officielle avec téléchargement public

Pour partager le plugin avec une vraie page de téléchargement :

1. Dans ton dépôt, clique **Releases** (colonne de droite) → **Create a new release**
2. Dans **Choose a tag** → tape `v1.0.0` → **Create new tag**
3. Titre : `VCA Compressor v1.0.0`
4. Clique **Publish release**

➡ GitHub Actions va automatiquement recompiler et **attacher le .exe à la Release**.
   N'importe qui pourra le télécharger depuis ta page GitHub !

---

## FAQ

**Q : Est-ce que c'est vraiment gratuit ?**
Oui. GitHub Actions offre 2000 minutes/mois sur les dépôts publics.
Chaque build prend ~10 min. Tu as donc ~200 builds/mois gratuits.

**Q : Mes sources sont-elles publiques ?**
Si tu mets le dépôt en **Public**, oui. Pour le garder privé avec Actions
gratuit, tu peux utiliser 500 min/mois offertes sur les dépôts privés.

**Q : Le plugin fonctionnera sur quel Windows ?**
Windows 10 et 11, 64 bits uniquement (standard actuel pour VST3).

**Q : Compatible avec quels DAW ?**
Tout DAW supportant le standard **VST3** :
Cubase, Nuendo, Ableton Live 11+, FL Studio 21+, Reaper, Studio One,
Bitwig Studio, Cakewalk, Logic Pro (via wrapper), Pro Tools (via AAX wrapper).

---

## Modifier la version

Pour bumper la version à `v1.1.0` :
1. Edite `source/version.h` → change `MAJOR_VERSION_STR`, `MINOR_VERSION_STR`…
2. Edite le script Inno Setup dans `build.yml` → change `AppVersion` et `OutputBaseFilename`
3. Commit → crée un tag `v1.1.0` → nouvelle Release automatique
