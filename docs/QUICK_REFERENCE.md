# Documentation Quick Reference

Quick commands and tips for working with the R-TYPE J.A.M.E.S. documentation.

## 🚀 Quick Start

```bash
# First time setup
cd docs
npm install

# Start development server
npm start
```

Visit: http://localhost:3000/R_TYPE_J.A.M.E.S/

## 📝 Common Tasks

### View Documentation Locally
```bash
cd docs && npm start
```

### Build for Production
```bash
cd docs && npm run build
```

### Test Production Build
```bash
cd docs && npm run build && npm run serve
```

### Clear Cache (if having issues)
```bash
cd docs && npm run clear
```

## 📄 Adding New Documentation

### Create a New Page

1. Create a new `.md` file in `docs/docs/`:
```bash
touch docs/docs/my-new-page.md
```

2. Add front matter:
```markdown
---
sidebar_position: 4
---

# My New Page

Content here...
```

3. It will automatically appear in the sidebar!

### Organizing Pages in Folders

```
docs/docs/
├── intro.md
├── architecture.md
├── protocol.md
└── guides/
    ├── _category_.json
    ├── getting-started.md
    └── advanced.md
```

Create `_category_.json`:
```json
{
  "label": "Guides",
  "position": 3,
  "link": {
    "type": "generated-index"
  }
}
```

## 🎨 Markdown Features

### Admonitions (Callouts)

```markdown
:::note
This is a note
:::

:::tip
Helpful tip here
:::

:::info
Information box
:::

:::warning
Warning message
:::

:::danger
Danger alert
:::
```

### Code Blocks with Highlighting

````markdown
```cpp title="src/example.cpp" {1,3-5}
#include <iostream>

int main() {
    std::cout << "Hello, R-TYPE!" << std::endl;
    return 0;
}
```
````

### Tabs

```markdown
import Tabs from '@theme/Tabs';
import TabItem from '@theme/TabItem';

<Tabs>
  <TabItem value="linux" label="Linux" default>
    ```bash
    ./build/server/r-type_server
    ```
  </TabItem>
  <TabItem value="windows" label="Windows">
    ```bash
    .\build\server\r-type_server.exe
    ```
  </TabItem>
</Tabs>
```

### Links

```markdown
[Internal Link](./architecture.md)
[External Link](https://github.com/MatthieuGA/R_TYPE_J.A.M.E.S.)
```

## 🔧 Configuration

### Update Site Metadata
Edit `docs/docusaurus.config.ts`:
- `title`: Site title
- `tagline`: Subtitle
- `url`: Production URL
- `baseUrl`: Base path

### Customize Theme
Edit `docs/src/css/custom.css`:
```css
:root {
  --ifm-color-primary: #2e8555;
  --ifm-color-primary-dark: #29784c;
  /* ... */
}
```

## 🚢 Deployment

### Manual Deploy
```bash
cd docs
GIT_USER=<your-github-username> npm run deploy
```

### Automatic (GitHub Actions)
Just push to `main` branch. The workflow at `.github/workflows/deploy-docs.yml` will:
1. Build the documentation
2. Deploy to GitHub Pages

## 📊 File Structure Reference

```
docs/
├── docs/               # Documentation pages (.md files)
├── blog/              # Blog posts (optional)
├── src/
│   ├── components/    # Custom React components
│   ├── css/          # Custom styles
│   └── pages/        # Custom pages (e.g., home)
├── static/           # Static assets (images, files)
├── docusaurus.config.ts  # Main config
├── sidebars.ts       # Sidebar structure
└── package.json      # Dependencies
```

## 🐛 Troubleshooting

### Clear cache and reinstall
```bash
cd docs
rm -rf node_modules package-lock.json .docusaurus
npm install
npm start
```

### Port already in use
```bash
npm start -- --port 3001
```

### Build fails
```bash
npm run clear
npm run build
```

## 📚 Learn More

- [Docusaurus Documentation](https://docusaurus.io/)
- [Markdown Features](https://docusaurus.io/docs/markdown-features)
- [React Components](https://docusaurus.io/docs/markdown-features/react)

## ✅ Checklist for New Contributors

- [ ] Install Node.js 20+
- [ ] Run `npm install` in docs/
- [ ] Start dev server with `npm start`
- [ ] Make your changes
- [ ] Test locally
- [ ] Build with `npm run build` to verify
- [ ] Commit and push
