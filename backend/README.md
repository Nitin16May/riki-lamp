# RiKi Lamp Backend State Retainer

Since the physical lamps have already been shipped with non-retaining MQTT firmware, this lightweight backend daemon runs in the background 24/7 to intercept any live color/effect changes sent by the physical lamps, and immediately republishes them with the `retain` flag set to `true`.

This ensures that:
1. When you open the web dashboard at any time, it immediately shows the correct latest state of the lamps.
2. If a physical lamp boots up or reconnects, it immediately syncs to its partner's latest state.

## How to Run Locally

### 1. Install Dependencies
Ensure you have Python 3 installed. Install the required MQTT library:
```bash
pip install -r requirements.txt
```

### 2. Run the Script
Run the retainer script:
```bash
python retainer.py
```

---

## How to Run 24/7 (Free Hosting Options)

To avoid keeping your personal computer running 24/7, you can deploy this script to a free cloud hosting service like **Render**, **Railway**, or **Fly.io** as a background worker process.

### Option A: Render (Free Background Worker)
1. Push this repository to GitHub.
2. Sign up on [Render.com](https://render.com) and click **New** -> **Background Worker**.
3. Link your GitHub repository.
4. Set the following settings:
   * **Language**: `Python`
   * **Build Command**: `pip install -r backend/requirements.txt`
   * **Start Command**: `python backend/retainer.py`
5. Click deploy! It will run in the background 24/7 for free.

### Option B: Railway (Free Trial)
1. Push this repository to GitHub.
2. Connect your repository on [Railway.app](https://railway.app).
3. Railway will automatically detect the Python project and run it as a continuous background daemon!
