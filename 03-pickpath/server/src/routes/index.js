const express = require('express');
const { protect, requireRole } = require('../middleware/auth');
const auth = require('../controllers/authController');
const bins = require('../controllers/binController');
const pickLists = require('../controllers/pickListController');
const { getWarehouse } = require('../controllers/warehouseController');

const authRouter = express.Router();
authRouter.post('/login', auth.login);
authRouter.post('/badge', auth.badgeLogin);
authRouter.get('/me', protect, auth.me);
authRouter.patch('/pin', protect, auth.changePin);

const binRouter = express.Router();
binRouter.use(protect);
binRouter.get('/', bins.listBins);
binRouter.patch('/:id', requireRole('lead'), bins.updateBin);

const pickListRouter = express.Router();
pickListRouter.use(protect);
pickListRouter.get('/', pickLists.listPickLists);
pickListRouter.post('/', requireRole('lead'), pickLists.createPickList);
pickListRouter.post('/:id/route', pickLists.planRoute);

const warehouseRouter = express.Router();
warehouseRouter.get('/', protect, getWarehouse);

module.exports = { authRouter, binRouter, pickListRouter, warehouseRouter };
