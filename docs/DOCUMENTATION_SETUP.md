# Documentation Setup - R-TYPE J.A.M.E.S.

This document describes the Docusaurus documentation setup for the R-TYPE J.A.M.E.S. project.

## What Was Done

### 1. Docusaurus Installation
- Initialized Docusaurus v3 with TypeScript support
- Configured for GitHub Pages deployment
- Set up project metadata (title, tagline, URLs)

### 2. Documentation Pages Created

#### Getting Started (`docs/intro.md`)
- Prerequisites and requirements
- Quick start guide
- Build and run instructions
- Running tests

#### Architecture (`docs/architecture.md`)
- System architecture overview
- Client-server model
- Entity Component System (ECS) explanation
- Core components description
- Directory structure
- Data flow diagrams
- Threading model
- Performance considerations

#### Protocol (`docs/protocol.md`)
- Network protocol specification
- UDP communication details
- Packet structures and formats
- Client-server communication flow
- Reliability mechanisms
- Client-side prediction
- Bandwidth optimization
- Security considerations

### 3. Configuration Updates

#### Docusaurus Config (`docusaurus.config.ts`)
- Updated site title and tagline
- Configured GitHub Pages deployment
  - Organization: MatthieuGA
  - Repository: R_TYPE_J.A.M.E.S.
  - Base URL: /R_TYPE_J.A.M.E.S./
- Updated navigation items
- Customized footer links
- Set up GitHub repository links

#### Home Page (`src/pages/index.tsx`)
- Updated button text to "Get Started - Documentation"
- Kept the Docusaurus classic template design

### 4. GitHub Actions Workflow

Created `.github/workflows/deploy-docs.yml`:
- Triggers on push to main branch
- Builds documentation with Node.js 20
- Deploys to GitHub Pages
- Automatic deployment on documentation changes

### 5. Main README Update

Updated `/README.md` with:
- Link to online documentation
- Instructions for running docs locally
- Prerequisites (Node.js 20+)
- Commands for `npm install`, `npm start`, `npm run build`, `npm run serve`
- Complete project documentation structure
- Development workflow integration

## File Structure

```
R_TYPE_J.A.M.E.S./
├── docs/                           # Docusaurus root
│   ├── .gitignore                  # Node modules and build artifacts
│   ├── docusaurus.config.ts        # Main configuration
│   ├── package.json                # Dependencies
│   ├── sidebars.ts                 # Sidebar configuration
│   ├── tsconfig.json               # TypeScript config
│   ├── docs/                       # Documentation pages
│   │   ├── intro.md                # Getting Started
│   │   ├── architecture.md         # Architecture guide
│   │   └── protocol.md             # Protocol specification
│   ├── src/                        # React components
│   │   ├── pages/                  # Custom pages
│   │   │   └── index.tsx           # Home page
│   │   ├── components/             # React components
│   │   └── css/                    # Custom styles
│   └── static/                     # Static assets
│       └── img/                    # Images
├── .github/
│   └── workflows/
│       └── deploy-docs.yml         # GitHub Pages deployment
└── README.md                       # Updated with docs info
```

## Commands Reference

### Development
```bash
cd docs
npm install          # Install dependencies (first time)
npm start            # Start dev server at http://localhost:3000
```

### Production Build
```bash
cd docs
npm run build        # Build static site to docs/build/
npm run serve        # Serve production build locally
```

### Deployment
```bash
cd docs
npm run deploy       # Deploy to GitHub Pages (requires GIT_USER)
```

Or use GitHub Actions (automatic on push to main).

## GitHub Pages Setup

To enable GitHub Pages:

1. Go to repository Settings → Pages
2. Source: Select "GitHub Actions"
3. The workflow will automatically deploy on push to main

The site will be available at: `https://matthieuga.github.io/R_TYPE_J.A.M.E.S/`

## Next Steps for Enhancement

### Short-term
- [ ] Add code API documentation (Doxygen integration)
- [ ] Add diagrams for system architecture
- [ ] Create tutorial for adding new components
- [ ] Document build system in detail

### Long-term
- [ ] Add interactive code examples
- [ ] Create video tutorials
- [ ] Add troubleshooting section
- [ ] Multilingual support (if needed)
- [ ] Search functionality (Algolia DocSearch)

## Maintenance

### Updating Documentation
1. Edit files in `docs/docs/`
2. Test locally with `npm start`
3. Commit and push to trigger automatic deployment

### Adding New Pages
1. Create `.md` file in `docs/docs/`
2. Add front matter with `sidebar_position`
3. Update `sidebars.ts` if needed

### Customization
- Modify `docusaurus.config.ts` for site-wide settings
- Edit `src/css/custom.css` for styling
- Create React components in `src/components/`

## Dependencies

- Docusaurus: v3.x (latest)
- Node.js: v20.x or higher
- React: v18.x
- TypeScript: v5.x

## Known Issues

None at this time.

## Contact

For documentation issues or suggestions, please open an issue on GitHub.
